#include "basecommand.h"
#include <hemeradevelopermodetargetmanager.h>
#include <hemeradevelopermodestar.h>
#include <hemeradevelopermodecontroller.h>
#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodeoperation.h>
#include <transports/hemeradevelopermodetransport.h>
#include <iostream>

#include <QtCore/QDebug>

BaseCommand::BaseCommand(QObject *parent)
    : QObject(parent)
    , m_maxCommandSize(0)
    , m_signalHandlingBehaviors(DefaultSignalHandlingBehavior)
{
}

void BaseCommand::addSubcommand(BaseCommand *command)
{
    if (!command) {
        return;
    }

    if (m_subcommands.contains(command->name())) {
        qWarning() << "Command" << command->name() << "is already registered as subcommand of" << name();
        return;
    }

    m_subcommands[command->name()] = command;
    connect(command, &BaseCommand::finished, this, &BaseCommand::finished);

    if (m_maxCommandSize < command->name().size())
        m_maxCommandSize = command->name().size();
}

void BaseCommand::execute(const QStringList &arguments, int depth)
{
    if (arguments.size() >= depth + 1) {
        QString firstArg = arguments.at(depth);

        if (isSubcommand(firstArg)) {
            m_runningSubcommand = subcommand(firstArg);
            subcommand(firstArg)->execute(arguments, depth + 1);
            return;
        }
    }

    QString applicationDescription = "\n" + longDescription();
    if (m_subcommands.size() > 0) {
        applicationDescription.append("\n\nCommands: (run each command with --help for more info)");
        for (QMap<QString, BaseCommand*>::const_iterator i = m_subcommands.begin(); i != m_subcommands.end(); ++i) {
            applicationDescription.append(QString("\n  %1  %2").arg(i.key(), m_maxCommandSize).arg(i.value()->briefDescription()));
        }
    }

    // filter arguments removing commands
    // we need to leave the first one (executable) in its place or QCommandLineParser won't work

    QStringList filteredArgs = arguments;
    for(int i = 1; i < depth && filteredArgs.size() > 1; ++i) {
        filteredArgs.removeAt(1);
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(applicationDescription);
    parser.addHelpOption();

    setupParser(&parser, filteredArgs);

    // we don't use 'process' because it can't handle well help string for subcommands
    // so we're going to manually fix and print help string later
    parser.parse(filteredArgs);

    QStringList optionNames = parser.optionNames();
    if (parser.positionalArguments().isEmpty() && optionNames.size() == 1 && (optionNames.contains("h") || optionNames.contains("help"))) {
        QString helpString = parser.helpText();

        // fix usage line for subcommands
        QString usageString = "Usage: " + arguments.at(0);
        if (depth > 1) {
            QString usageFullString = "Usage: " + arguments.at(0);
            for (int i = 1; i < depth; ++i) {
                usageFullString += " " + arguments.at(i);
            }
            helpString.replace(usageString, usageFullString);
            if (hasSubcommands()) {
                helpString.prepend(usageFullString + " command\n");
            }
        } else if (hasSubcommands()) {
            helpString.prepend(usageString + " command\n");
        }

        std::cout << helpString.toStdString();
        Q_EMIT finished(0);
    } else {
        parseAndExecute(&parser, filteredArgs);
    }
}

bool BaseCommand::hasSubcommands()
{
    return !m_subcommands.isEmpty();
}

bool BaseCommand::isSubcommand(const QString &name)
{
    return m_subcommands.contains(name);
}

bool BaseCommand::parseAndExecute(QCommandLineParser *parser, const QStringList &arguments)
{
    Q_UNUSED(parser)
    Q_UNUSED(arguments);
    qWarning() << "Default parseAndExecute does nothing";
    Q_EMIT finished(0);
    return true;
}

void BaseCommand::printErrorAndExit(const QString &errorMessage, int exitStatus)
{
    std::cerr << errorMessage.toStdString() << std::endl;
    Q_EMIT finished(exitStatus);
}

void BaseCommand::setupParser(QCommandLineParser *parser, const QStringList &arguments)
{
    Q_UNUSED(parser);
    Q_UNUSED(arguments);
}

BaseCommand::SignalHandlingBehaviors BaseCommand::signalHandlingBehaviors() const
{
    SignalHandlingBehaviors combinedBehaviors = m_signalHandlingBehaviors;
    if (!m_subcommands.isEmpty()) {
        for (QMap<QString, BaseCommand*>::const_iterator i = m_subcommands.constBegin(); i  != m_subcommands.constEnd(); ++i) {
            combinedBehaviors |= i.value()->signalHandlingBehaviors();
        }
    }

    return combinedBehaviors;
}

void BaseCommand::setSignalHandlingBehaviors(SignalHandlingBehaviors behaviors)
{
    m_signalHandlingBehaviors = behaviors;
}

bool BaseCommand::onTermRequest()
{
    // Try delegating to subcommand, if any.
    if (!m_runningSubcommand.isNull()) {
        if (m_runningSubcommand->onTermRequest()) {
            return true;
        }
    }

    return false;
}

QStringList BaseCommand::subcommands()
{
    return m_subcommands.keys();
}

BaseCommand* BaseCommand::subcommand(const QString &name)
{
    if (! isSubcommand(name)) {
        return 0;
    }
    return m_subcommands[name];
}

BaseCommand* BaseCommand::removeSubcommand(const QString &name)
{
    if (! isSubcommand(name)) {
        return 0;
    }
    BaseCommand *command = m_subcommands.take(name);
    disconnect(command, &BaseCommand::finished, this, &BaseCommand::finished);
    return command;
}

Hemera::DeveloperMode::Target::Ptr BaseCommand::retrieveTargetOrDie(const QString& targetName, bool connected)
{
    Hemera::DeveloperMode::Target::Ptr target = Hemera::DeveloperMode::TargetManager::instance()->loadTarget(targetName);
    if (targetName.isEmpty() || target.isNull()) {
        printErrorAndExit(tr("Unknown Hemera target \"%1\"").arg(targetName));
        return Hemera::DeveloperMode::Target::Ptr();
    }

    if (connected) {
        // Now, wait for target to come up.
        if (!target->waitForTargetInfo()) {
            printErrorAndExit(tr("Target \"%1\" appears to be offline, or doesn't have DeviceInfo available.").arg(targetName));
            return Hemera::DeveloperMode::Target::Ptr();
        }
    }

    return target;
}

Hemera::DeveloperMode::Device::Ptr BaseCommand::retrieveDeviceOrDie(const QString &deviceName, bool connected)
{
    Hemera::DeveloperMode::Device::Ptr device = Hemera::DeveloperMode::TargetManager::instance()->loadDevice(deviceName);
    if (deviceName.isEmpty() || device.isNull()) {
        printErrorAndExit(tr("Unknown Hemera device \"%1\"").arg(deviceName));
        return Hemera::DeveloperMode::Device::Ptr();
    }

    if (connected) {
        // Now, wait for target to come up.
        if (!device->waitForTargetInfo()) {
            printErrorAndExit(tr("Device \"%1\" appears to be offline, or doesn't have DeviceInfo available.").arg(deviceName));
            return Hemera::DeveloperMode::Device::Ptr();
        }
    }

    return device;
}

Hemera::DeveloperMode::Emulator::Ptr BaseCommand::retrieveEmulatorOrDie(const QString &emulatorName, bool connected)
{
    using namespace Hemera::DeveloperMode;
    Emulator::Ptr emulator = TargetManager::instance()->loadEmulator(emulatorName);

    if (!emulator) {
        printErrorAndExit(tr("No such emulator \"%1\".").arg(emulatorName));
        return Hemera::DeveloperMode::Emulator::Ptr();
    }

    if (connected) {
        // Wait for it to start...
        bool wasRunning = emulator->isRunning();
        Hemera::DeveloperMode::Operation *operation = emulator->start();
        if (!operation->synchronize(5000)) {
            printErrorAndExit(tr("Emulator \"%1\" could not be started. %2: %3").arg(emulator->name(), operation->errorName(), operation->errorMessage()));
            return Hemera::DeveloperMode::Emulator::Ptr();
        }

        // Now, wait for target to come up.
        if (!wasRunning) {
            std::cout << tr("The emulator/compiler is starting. This might take a while...").toStdString() << std::endl;
        }
        if (!emulator->waitForTargetInfo(30000)) {
            printErrorAndExit(tr("Target \"%1\" appears to be offline, or doesn't have DeviceInfo available.").arg(emulator->name()));
            return Hemera::DeveloperMode::Emulator::Ptr();
        }
    }

    return emulator;
}

Hemera::DeveloperMode::Emulator::Ptr BaseCommand::retrieveAnyEmulatorOrDie(bool connected)
{
    using namespace Hemera::DeveloperMode;
    // If there's a running emulator, it's set
    if (TargetManager::instance()->runningEmulator()) {
        Emulator::Ptr emulator = TargetManager::instance()->runningEmulator();
        if (connected) {
            if (!emulator->waitForTargetInfo(5000)) {
                printErrorAndExit(tr("Emulator \"%1\" appears to be running, but a connection attempt failed. "
                                        "This should not happen, your emulator might be broken, please reinstall and restart it.").arg(emulator->name()));
            }
        }
        return emulator;
    }

    // Otherwise just load one at random.
    if (TargetManager::registeredEmulators().isEmpty()) {
        printErrorAndExit(tr("There are no emulators registered on this system yet. Add one with hsdk emulator install."));
        return Hemera::DeveloperMode::Emulator::Ptr();
    }

    return retrieveEmulatorOrDie(TargetManager::registeredEmulators().keys().first(), connected);
}

Hemera::DeveloperMode::Emulator::Ptr BaseCommand::retrieveEmulatorForDeviceOrDie(const QString &deviceName, bool connected)
{
    Hemera::DeveloperMode::Device::Ptr device = retrieveDeviceOrDie(deviceName, false);
    if (!device) {
        return Hemera::DeveloperMode::Emulator::Ptr();
    }

    Hemera::DeveloperMode::Emulator::Ptr emulator = device->associatedEmulator();
    if (!emulator) {
        printErrorAndExit(tr("Device %1 has no associated emulator yet. Associate one with hsdk device associate.").arg(deviceName));
        return Hemera::DeveloperMode::Emulator::Ptr();
    }

    return retrieveEmulatorOrDie(emulator->name(), connected);
}

Hemera::DeveloperMode::Star* BaseCommand::retrieveStarOrDie(Hemera::DeveloperMode::Target::Ptr target, const QString& s)
{
    if (!target->stars().contains(s)) {
        printErrorAndExit(tr("Specified Star '%1' does not exist on target '%2'. Available stars are: %3.").arg(s, target->name(),
                                                                                                                target->stars().join(", ")));
        return Q_NULLPTR;
    }

    Hemera::DeveloperMode::Star *star = target->star(s);

    if (star == Q_NULLPTR) {
        printErrorAndExit(tr("Internal error while retrieving Star '%1' on target '%2'. The Star might not exist.").arg(s, target->name()));
        return Q_NULLPTR;
    }

    // Wait for its validity.
    if (!star->waitForValid()) {
        printErrorAndExit(tr("Could not retrieve a valid Star '%1' for target '%2'.").arg(s, target->name()));
        return Q_NULLPTR;
    }

    return star;
}

Hemera::DeveloperMode::Star* BaseCommand::retrieveStarOrDie(const QString& targetName, const QString& star)
{
    Hemera::DeveloperMode::Target::Ptr target = retrieveTargetOrDie(targetName);
    if (!target) {
        return Q_NULLPTR;
    }

    return retrieveStarOrDie(target, star);
}

Hemera::DeveloperMode::Controller::Ptr BaseCommand::retrieveDeveloperModeControllerOrDie(Hemera::DeveloperMode::Target::Ptr target)
{
    // Wait for target to come up.
    if (!target->waitForTargetInfo()) {
        printErrorAndExit(tr("Target \"%1\" appears to be offline, or doesn't have DeviceInfo available.").arg(target->name()));
        return Hemera::DeveloperMode::Controller::Ptr();
    }

    Hemera::DeveloperMode::Device::Ptr device = qobject_cast<Hemera::DeveloperMode::Device::Ptr>(target);
    if (device) {
        if (device->isProductionDevice()) {
            printErrorAndExit(tr("Target \"%1\" appears to be a production device, developer mode won't be available here.").arg(target->name()));
            return Hemera::DeveloperMode::Controller::Ptr();
        }
    }

    if (!target->hasAcquiredDeveloperModeController()) {
        if (!target->ensureDeveloperModeController()->synchronize()) {
            printErrorAndExit(tr("Could not retrieve developer mode controller for target '%1'.").arg(target->name()));
            return Hemera::DeveloperMode::Controller::Ptr();
        }
    }

    return target->developerModeController();
}

Hemera::DeveloperMode::Controller::Ptr BaseCommand::retrieveDeveloperModeControllerOrDie(const QString& targetName)
{
    Hemera::DeveloperMode::Target::Ptr target = retrieveTargetOrDie(targetName);
    if (!target) {
        return Hemera::DeveloperMode::Controller::Ptr();
    }

    return retrieveDeveloperModeControllerOrDie(target);
}

Hemera::DeveloperMode::Target::Ptr BaseCommand::loadTarget(QCommandLineParser *parser, QCommandLineOption emulatorOption, QCommandLineOption deviceOption,
                                                           bool connected, bool retrieveDefaultTarget)
{
    Hemera::DeveloperMode::Target::Ptr target;
    if (parser->isSet(emulatorOption) && parser->isSet(deviceOption)) {
        printErrorAndExit("You can't use \"emulator\" and \"device\" options together. Use --help flag for usage info.");
    } else if (parser->isSet(deviceOption)) {
        target = retrieveDeviceOrDie(parser->value(deviceOption), connected);
    } else if (parser->isSet(emulatorOption) || retrieveDefaultTarget) {
        if (!parser->value(emulatorOption).isEmpty()) {
            target = retrieveEmulatorOrDie(parser->value(emulatorOption));
        } else {
            target = retrieveAnyEmulatorOrDie(connected);
        }
    } else {
        printErrorAndExit(tr("No target specified. Use --help flag for usage info."));
    }

    return target;
}

Hemera::DeveloperMode::Emulator::Ptr BaseCommand::loadEmulatorEnvironment(QCommandLineParser* parser, QCommandLineOption emulatorOption,
                                                                          QCommandLineOption deviceOption, bool connected)
{
    Hemera::DeveloperMode::Emulator::Ptr target;

    if (parser->isSet(emulatorOption) && parser->isSet(deviceOption)) {
        printErrorAndExit("You can't use \"emulator\" and \"device\" options together. Use --help flag for usage info.");
    } else if (parser->isSet(deviceOption)) {
        // Retrieve emulator for specific device
        target = retrieveEmulatorForDeviceOrDie(parser->value(deviceOption), connected);
    } else {
        if (parser->value(emulatorOption).isEmpty()) {
            // Load any emulator
            target = retrieveAnyEmulatorOrDie(connected);
        } else {
            target = retrieveEmulatorOrDie(parser->value(emulatorOption), connected);
        }
    }

    return target;
}

