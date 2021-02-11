#include "emulatorcommand.h"

#include <hemeradevelopermodetargetmanager.h>
#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodeoperation.h>

#include <QtCore/QFile>
#include <QtCore/QDebug>

#include <iostream>

EmulatorCommand::EmulatorCommand(QObject *parent)
    : BaseCommand(parent)
{
    addSubcommand(new InstallEmulatorCommand());
    addSubcommand(new RemoveEmulatorCommand());
    addSubcommand(new ListEmulatorsCommand());
    addSubcommand(new ListVMCommand());
    addSubcommand(new StartEmulatorCommand);
    addSubcommand(new StopEmulatorCommand);
}

EmulatorCommand::~EmulatorCommand()
{
}

QString EmulatorCommand::longDescription()
{
    return tr("Manage Emulators");
}

QString EmulatorCommand::briefDescription()
{
    return tr("Manage Emulators");
}

QString EmulatorCommand::name()
{
    return QStringLiteral("emulator");
}

////////////////////////////////////////////////////

InstallEmulatorCommand::InstallEmulatorCommand(QObject* parent)
    : BaseCommand(parent)
    , m_keepOption(QStringList() << "k" << "keep", tr("If the emulator already exists, keeps the old hard disk instead of deleting it. Disabled by default."))
    , m_moveVDIOption("no-move", tr("When installing from a VDI file, do not move the VDI into its standard location. Disabled by default."))
    , m_startServerOption("start-server", tr("When installing from a Start token, specify a custom Start server"), "start-server", "https://start.hemera.io")
{
}

InstallEmulatorCommand::~InstallEmulatorCommand()
{

}

QString InstallEmulatorCommand::briefDescription()
{
    return tr("Installs or updates an existing emulator from a known source.");
}

QString InstallEmulatorCommand::longDescription()
{
    return tr("Installs or updates an existing emulator from a known source. When invoked without "
              "arguments, it simply verifies everything is set up properly for the virtual machine and "
              "all the needed hooks are in place, if the emulator exists.\n\n"
              "It can be given a path to a VDI file which will then be used "
              "as the main emulator disk. The previous VDI file can be kept with the --keep option. "
              "By default, such a file will be then moved to a known location - in case this is not desirable, "
              "use the --no-move option.\n\n"
              "It can also be given a Start token (plus an additional Start server argument) for automated "
              "Emulator deployment if you are using Start.");
}

QString InstallEmulatorCommand::name()
{
    return QStringLiteral("install");
}

bool InstallEmulatorCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    QStringList positionalArguments = parser->positionalArguments();
    if (positionalArguments.size() != 2 && positionalArguments.size() != 1) {
        printErrorAndExit(tr("Wrong required parameters. Use --help flag for usage info."));
        return false;
    }

    QString name = positionalArguments.at(0);
    QString location;
    if (positionalArguments.size() > 1) {
        location = positionalArguments.at(1);
    }

    using namespace Hemera::DeveloperMode;
    TargetManager::EmulatorInstallModes modes = TargetManager::EmulatorInstallMode::NoMode;
    if (parser->isSet(m_keepOption)) {
        modes |= TargetManager::EmulatorInstallMode::KeepExistingVDI;
    }
    if (parser->isSet(m_moveVDIOption)) {
        modes |= TargetManager::EmulatorInstallMode::MoveVDI;
    }

    // Verify what we are talking about.
    Operation *op = Q_NULLPTR;
    if (location.isEmpty()) {
        // Ok, the emulator gotta exist then.
        if (!TargetManager::instance()->registeredEmulators().contains(name)) {
            printErrorAndExit(tr("Requested update of emulator \"%1\", but such an emulator does not exist!").arg(name));
            return false;
        }
    } else if (QFile::exists(location)) {
        // It's gotta be a VDI file
        op = TargetManager::instance()->installEmulatorFromVDI(name, location, QCoreApplication::applicationFilePath(), modes);
    } else {
        printErrorAndExit(tr("This command currently works with local VDIs only."));
        return false;
    }

    std::cout << tr("Installing emulator %1...").arg(name).toStdString() << "\r";
    std::cout.flush();

    if (op) {
        connect(op, &Operation::finished, [this, op, name] {
            // Escape carriage return
            std::cout << std::endl;

            if (op->isError()) {
                printErrorAndExit(tr("Installation failed! %1: %2.").arg(op->errorName(), op->errorMessage()));
                Q_EMIT finished(1);
                return;
            }

            std::cout << "\n" << tr("Installation succeeded! You can now associate %1 to any of your devices.").arg(name).toStdString() << std::endl;
            Q_EMIT finished(0);
        });
        connect(op, &Operation::progress, [this, op, name] (int percent) {
            std::cout << tr("Installing emulator %1... %2%").arg(name).arg(percent).toStdString() << "\r";
            std::cout.flush();
        });
        connect(op, &Operation::downloadInfo, [this, op, name] (quint64 downloaded, quint64 total, quint64 rate) {
            // TODO: Handle me!
            std::cout << tr("Downloading emulator... %1%, %2 %3 of %4 %5 (%6 %7/s)").toStdString() << "\r";
            std::cout.flush();
        });
    }

    return true;
}

void InstallEmulatorCommand::setupParser(QCommandLineParser* parser, const QStringList& arguments)
{
    parser->addOption(m_keepOption);
    parser->addOption(m_moveVDIOption);
    parser->addOption(m_startServerOption);
    parser->addPositionalArgument(QStringLiteral("name"), tr("Name of the emulator to install."));
    parser->addPositionalArgument(QStringLiteral("vdi_path_or_token"), tr("Path to a VDI file, or a valid token for Start."));
}


RemoveEmulatorCommand::RemoveEmulatorCommand(QObject* parent)
    : BaseCommand(parent)
    , m_keepOption(QStringList() << "k" << "keep", tr("Keeps all the Emulator's files, and only unregisters it. Disabled by default."))
{
}

RemoveEmulatorCommand::~RemoveEmulatorCommand()
{
}

QString RemoveEmulatorCommand::briefDescription()
{
    return tr("Removes an existing emulator from the system.");
}

QString RemoveEmulatorCommand::longDescription()
{
    return tr("Removes an existing emulator from the system. Unless --keep is specified, all of "
              "its associated files are deleted as well.");
}

QString RemoveEmulatorCommand::name()
{
    return QStringLiteral("remove");
}

bool RemoveEmulatorCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    QStringList positionalArguments = parser->positionalArguments();
    if (positionalArguments.size() != 1) {
        printErrorAndExit(tr("Wrong required parameters. Use --help flag for usage info."));
        return false;
    }

    QString name = positionalArguments.at(0);
    Hemera::DeveloperMode::Operation *op = Hemera::DeveloperMode::TargetManager::instance()->removeEmulator(name, parser->isSet(m_keepOption));
    if (op) {
        connect(op, &Hemera::DeveloperMode::Operation::finished, [this, op, name] {
            if (op->isError()) {
                printErrorAndExit(tr("Could not remove emulator! %1: %2.").arg(op->errorName(), op->errorMessage()));
                return;
            }

            std::cout << tr("Emulator successfully removed.").toStdString() << std::endl;
            Q_EMIT finished(0);
        });
    } else {
        printErrorAndExit(tr("Could not initiate removal of emulator \"%1\"!").arg(name));
        return false;
    }

    return true;
}

void RemoveEmulatorCommand::setupParser(QCommandLineParser* parser, const QStringList& arguments)
{
    parser->addOption(m_keepOption);
    parser->addPositionalArgument(QStringLiteral("name"), tr("Name of the emulator to remove."));
}


StartEmulatorCommand::StartEmulatorCommand(QObject* parent)
    : BaseCommand(parent)
    , m_headlessOption(QStringLiteral("headless"), tr("Starts the emulator headlessly."))
{
}

StartEmulatorCommand::~StartEmulatorCommand()
{
}

QString StartEmulatorCommand::briefDescription()
{
    return tr("Starts an emulator.");
}

QString StartEmulatorCommand::longDescription()
{
    return tr("Starts an emulator. This also stops any other running emulator.");
}

QString StartEmulatorCommand::name()
{
    return QStringLiteral("start");
}

bool StartEmulatorCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    QStringList positionalArguments = parser->positionalArguments();
    if (positionalArguments.size() != 1) {
        printErrorAndExit(tr("Wrong required parameters. Use --help flag for usage info."));
        return false;
    }

    QString name = positionalArguments.at(0);

    Hemera::DeveloperMode::Emulator::Ptr emulator = Hemera::DeveloperMode::TargetManager::instance()->loadEmulator(name);
    if (!emulator) {
        printErrorAndExit(tr("Emulator \"%1\" does not exist!").arg(name));
        return false;
    }


    Hemera::DeveloperMode::Operation *op = emulator->start(parser->isSet(m_headlessOption));
    connect(op, &Hemera::DeveloperMode::Operation::finished, [this, op, name] {
        if (op->isError()) {
            printErrorAndExit(tr("Starting emulator failed! %1: %2.").arg(op->errorName(), op->errorMessage()));
            Q_EMIT finished(1);
            return;
        }

        Q_EMIT finished(0);
    });
}

void StartEmulatorCommand::setupParser(QCommandLineParser* parser, const QStringList& arguments)
{
    parser->addOption(m_headlessOption);
    parser->addPositionalArgument(QStringLiteral("name"), tr("Name of the emulator to start."));
}



StopEmulatorCommand::StopEmulatorCommand(QObject* parent)
    : BaseCommand(parent)
{
}

StopEmulatorCommand::~StopEmulatorCommand()
{
}

QString StopEmulatorCommand::briefDescription()
{
    return tr("Stops an emulator.");
}

QString StopEmulatorCommand::longDescription()
{
    return tr("Stops an emulator.");
}

QString StopEmulatorCommand::name()
{
    return QStringLiteral("stop");
}

bool StopEmulatorCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    QStringList positionalArguments = parser->positionalArguments();
    if (positionalArguments.size() != 1) {
        printErrorAndExit(tr("Wrong required parameters. Use --help flag for usage info."));
        return false;
    }

    QString name = positionalArguments.at(0);

    Hemera::DeveloperMode::Emulator::Ptr emulator = Hemera::DeveloperMode::TargetManager::instance()->loadEmulator(name);
    if (!emulator) {
        printErrorAndExit(tr("Emulator \"%1\" does not exist!").arg(name));
        return false;
    }


    Hemera::DeveloperMode::Operation *op = emulator->stop();
    connect(op, &Hemera::DeveloperMode::Operation::finished, [this, op, name] {
        if (op->isError()) {
            printErrorAndExit(tr("Stopping emulator failed! %1: %2.").arg(op->errorName(), op->errorMessage()));
            Q_EMIT finished(1);
            return;
        }

        Q_EMIT finished(0);
    });
}

void StopEmulatorCommand::setupParser(QCommandLineParser* parser, const QStringList& arguments)
{
    parser->addPositionalArgument(QStringLiteral("name"), tr("Name of the emulator to stop."));
}



ListEmulatorsCommand::ListEmulatorsCommand(QObject* parent)
    : BaseCommand(parent)
{
}

ListEmulatorsCommand::~ListEmulatorsCommand()
{
}

QString ListEmulatorsCommand::briefDescription()
{
    return tr("Lists registered Hemera emulators on this PC.");
}

QString ListEmulatorsCommand::longDescription()
{
    return tr("Lists registered Hemera emulators on this PC.");
}

QString ListEmulatorsCommand::name()
{
    return QStringLiteral("list");
}

bool ListEmulatorsCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    QHash< QString, QString > vms = Hemera::DeveloperMode::TargetManager::instance()->registeredEmulators();

    for (QHash< QString, QString >::const_iterator i = vms.constBegin(); i != vms.constEnd(); ++i) {
        Hemera::DeveloperMode::Emulator::Ptr emulator = Hemera::DeveloperMode::TargetManager::instance()->loadEmulator(i.key());
        if (!emulator) {
            std::cerr << tr("Emulator %1 is registered, but seems not to be available from VirtualBox!").arg(i.key()).toStdString() << std::endl;
            continue;
        }
        std::cout << i.key().toStdString() << std::endl;
        std::cout << '\t' << tr("Builds for architectures:").toStdString() << " " << emulator->buildArchitectures().join(QStringLiteral(", ")).toStdString() << std::endl;
        std::cout << '\t' << tr("Running:").toStdString() << " "
                  << (emulator == Hemera::DeveloperMode::TargetManager::instance()->runningEmulator() ? tr("yes").toStdString() : tr("no").toStdString())
                  << std::endl << std::endl;
    }

    Q_EMIT finished(0);

    return true;
}



ListVMCommand::ListVMCommand(QObject* parent)
    : BaseCommand(parent)
{
}

ListVMCommand::~ListVMCommand()
{
}

QString ListVMCommand::briefDescription()
{
    return tr("Lists available (and unassociated) virtual machines on this PC.");
}

QString ListVMCommand::longDescription()
{
    return tr("Lists available (and unassociated) virtual machines on this PC.");
}

QString ListVMCommand::name()
{
    return QStringLiteral("list-available");
}

bool ListVMCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    QHash< QString, QString > vms = Hemera::DeveloperMode::TargetManager::instance()->availableVirtualMachines();
    QHash< QString, QString > registeredEmulators = Hemera::DeveloperMode::TargetManager::instance()->registeredEmulators();

    for (QHash< QString, QString >::const_iterator i = vms.constBegin(); i != vms.constEnd(); ++i) {
        std::cout << (registeredEmulators.values().contains(i.value()) ? tr("Available, registered:").toStdString() : tr("Available, unregistered:").toStdString())
                  << " " << i.value().toStdString() << " (" << i.key().toStdString() << ")" << std::endl;
    }

    Q_EMIT finished(0);

    return true;
}
