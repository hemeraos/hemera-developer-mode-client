/*
 *
 */

#include "launchcommand.h"

#include <hemeradevelopermodecontroller.h>
#include <hemeradevelopermodestar.h>
#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodetargetmanager.h>
#include <hemeradevelopermodeapplicationoutput.h>

#include <QtCore/QJsonObject>
#include <QtCore/QTimer>
#include <QtCore/QSocketNotifier>

#include <iostream>

LaunchCommand::LaunchCommand(QObject* parent)
    : BaseCommand(parent)
    , m_emulatorOption(QStringList() << "e" << "emulator", "Build for the emulator.", "emulator")
    , m_deviceOption(QStringList() << "d" << "device", "The Hemera device you want to build for.", "device")
    , m_debugOption("debug", "Launch and debug the application with the specified mode.", "debug_mode")
    , m_starOption(QStringList() << "s" << "star", "The Star onto which the application should be launched. Optional if there's only one.", "name")
    , m_timeoutTimer(new QTimer(this))
    , m_target(Q_NULLPTR)
    , m_shuttingDown(false)
    , m_isRunning(false)
{
    // Default timeout is 3 seconds.
    m_timeoutTimer->setInterval(3000);
    m_timeoutTimer->setSingleShot(true);

    connect(m_timeoutTimer, &QTimer::timeout, [this] { printErrorAndExit(tr("Operation timed out, exiting.")); });
}

LaunchCommand::~LaunchCommand()
{
}

QString LaunchCommand::briefDescription()
{
    return tr("Launches a standalone application onto a remote target");
}

QString LaunchCommand::longDescription()
{
    return tr("Launches an orbital Hemera application through developer mode onto a specific target.\n"
              "If the application is launched in the standard mode, the application output "
              "is displayed in the terminal. If launched in debug mode, it enters in gdb console. "
              "To terminate the application/developer mode instance, just CTRL+C the command.\n\n"
              "Note: do not attempt to use this command on a production board! This command relies "
              "on Hemera's Developer Mode, and there's no standard way to force an application onto a "
              "Hemera production target");
}

QString LaunchCommand::name()
{
    return "launch";
}

bool LaunchCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    Q_UNUSED(arguments);

    // Load the target
    if (parser->isSet(m_emulatorOption) && parser->isSet(m_deviceOption)) {
        printErrorAndExit("You can't use \"emulator\" and \"device\" options together. Use --help flag for usage info.");
        return false;
    }

    m_target = loadTarget(parser, m_emulatorOption, m_deviceOption, true, true);

    if (m_target.isNull()) {
        return false;
    }

    if (retrieveDeveloperModeControllerOrDie(m_target).isNull()) {
        return false;
    }

    QStringList positionalArguments = parser->positionalArguments();
    if (positionalArguments.size() < 1) {
        printErrorAndExit(tr("Missing required parameters. Use --help flag for usage info."));
        return false;
    }
    if (positionalArguments.size() > 1) {
        printErrorAndExit(tr("Too many parameters. Use --help flag for usage info."));
        return false;
    }

    if (parser->isSet(m_debugOption)) {
        QStringList availableModes = QStringList() << "gdb" << "valgrind" << "strace";
        m_debugMode = parser->value(m_debugOption);
        if (!availableModes.contains(m_debugMode)) {
            printErrorAndExit(tr("Invalid mode %1. Available debug modes are: %2.").arg(m_debugMode, availableModes.join(QLatin1Char(','))));
            return false;
        }
    }

    bool isStarSet = parser->isSet(m_starOption);
    QString starName = parser->value(m_starOption);

    m_applicationId = positionalArguments.at(0);

    if (!m_target->installedApps().contains(m_applicationId)) {
        printErrorAndExit(tr("Application '%1' does not exist on target '%2'.").arg(m_applicationId, m_target->name()));
        return false;
    }

    if (m_target->stars().count() > 1) {
        if (!isStarSet) {
            printErrorAndExit(tr("No orbit handler specified, and target '%1' has more than one. Available handlers are: %2.").arg(m_target->name(),
                                                                                                                                    m_target->stars().join(", ")));
            return false;
        }
        m_star = retrieveStarOrDie(m_target, starName);
    } else {
        // Easy.
        m_star = retrieveStarOrDie(m_target, m_target->stars().first());
    }

    if (m_star == Q_NULLPTR) {
        return false;
    }

    // Whew. We made it! Let's get the controller now.
    m_controller = retrieveDeveloperModeControllerOrDie(m_target);
    if (m_controller.isNull()) {
        return false;
    }

    // FIXME HACK WTF: Without this, either the server or the client gets the same fd and goes totally nuts.
    QEventLoop e;
    QTimer::singleShot(200, &e, SLOT(quit()));
    e.exec();

    // Application output!
    m_applicationOutput = new Hemera::DeveloperMode::ApplicationOutput(m_target, m_applicationId, this);
    connect(m_applicationOutput, &Hemera::DeveloperMode::ApplicationOutput::newMessage, [this] (const QDateTime &timestamp, const QJsonObject &message) {
        std::cout << "[" << timestamp.toString().toStdString() << "] " << message.value(QStringLiteral("message")).toString().toStdString() << std::endl;
    });

//     m_sshHandler->execute(m_target->name(), QString::fromLatin1("/usr/libexec/hemera/tools/hemera-debug-helper --mode=gdb "
//                                                                 "--with-application-holder --orbital-application %1").arg(m_applicationId));

    // We dont like any timeout when running in debug mode
    if (!parser->isSet(m_debugOption)) {
        // Monitor what's happening here...
        // Longer timeout
        m_timeoutTimer->start(10000);
    }
    connect(m_controller.data(), &Hemera::DeveloperMode::Controller::statusChanged, [this] {
        m_timeoutTimer->stop();
        if (m_controller->statusOf(m_star->name()) == Hemera::DeveloperMode::Controller::Status::Running) {
            std::cout << "[HSDK]: "
                        << tr("Developer mode started, application running.").toStdString()
                        << std::endl;

            // We are now running!
            m_isRunning = true;
        } else if (m_controller->statusOf(m_star->name()) == Hemera::DeveloperMode::Controller::Status::Stopped && !m_shuttingDown) {
            // Gravity has reverted.
            printErrorAndExit(tr("Gravity could not load Orbital Application '%1'.").arg(m_applicationId));
            return;
        } else if (m_controller->statusOf(m_star->name()) == Hemera::DeveloperMode::Controller::Status::Stopped && m_shuttingDown) {
            std::cout << "[HSDK]: "
                        << tr("Developer mode terminated successfully.").toStdString()
                        << std::endl;

            Q_EMIT finished(0);
            return;
        } else {
            // Restart timer and wait until it settles.
            m_timeoutTimer->start();
        }
    });

    QString debugPrefix;
    QString debugSuffix;

    if (m_debugMode == QStringLiteral("gdb")) {
        debugSuffix = QStringLiteral("--hold-start");
    } else if (m_debugMode == QStringLiteral("strace")) {
        debugPrefix = QStringLiteral("strace");
    } else if (m_debugMode == QStringLiteral("valgrind")) {
        debugPrefix = QStringLiteral("valgrind");
    }

    // Ready to go!
    m_controller->startSimple(m_star->name(), m_applicationId, !m_debugMode.isEmpty(), debugPrefix, debugSuffix);

    std::cout << "[HSDK]: "
                << tr("Starting application '%1' on target '%2', Orbit Handler '%3'").arg(m_applicationId, m_target->name(), m_star->name()).toStdString()
                << std::endl;

    return true;
}

void LaunchCommand::setupParser(QCommandLineParser* parser, const QStringList& arguments)
{
    Q_UNUSED(arguments);

    parser->addOption(m_deviceOption);
    parser->addOption(m_emulatorOption);
    parser->addOption(m_debugOption);
    parser->addOption(m_starOption);
    parser->addPositionalArgument("application_id", "ID of the Orbital Application to execute.");
}

bool LaunchCommand::onTermRequest()
{
    if (m_isRunning) {
        std::cout << "[HSDK]: "
              << tr("Terminating developer mode...").toStdString()
              << std::endl;

        // We're shutting down!
        m_shuttingDown = true;
        m_timeoutTimer->start(10000);
        m_controller->stop(m_star->name());

        // We handle this.
        return true;
    }

    return BaseCommand::onTermRequest();
}
