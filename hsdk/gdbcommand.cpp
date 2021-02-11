/*
 *
 */

#include "gdbcommand.h"

#include <hemeradevelopermodecontroller.h>
#include <hemeradevelopermodestar.h>
#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodetargetmanager.h>
#include <hemeradevelopermodeapplicationoutput.h>
#include <hemeradevelopermodeshelloperation.h>

#include <QtCore/QJsonObject>
#include <QtCore/QTimer>
#include <QtCore/QSocketNotifier>

#include <iostream>

GDBCommand::GDBCommand(QObject* parent)
    : BaseRemoteCommand(parent)
    , m_emulatorOption(QStringList() << "e" << "emulator", tr("Choose a specific emulator"), "emulator")
    , m_deviceOption(QStringList() << "d" << "device", tr("Choose a specific Hemera Device."), "device")
    , m_interfaceOption(QStringList() << "i" << "interface", tr("Use a debugging interface."), "interface")
{
    setSignalHandlingBehaviors(AllowMultipleSIGINT);
}

GDBCommand::~GDBCommand()
{
}

QString GDBCommand::briefDescription()
{
    return tr("Attach gdb on a remote hemera application or process");
}

QString GDBCommand::longDescription()
{
    return tr("Attach gdb on a remote hemera application or process");
}

QString GDBCommand::name()
{
    return QStringLiteral("gdb");
}

bool GDBCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    Q_UNUSED(arguments);

    if (!BaseRemoteCommand::parseAndExecute(parser, arguments)) {
        return false;
    }

    QStringList positionalArguments = parser->positionalArguments();
    if (positionalArguments.count() == 1) {
        m_applicationId = positionalArguments.at(0);
    }

    // Load the target
    Hemera::DeveloperMode::Target::Ptr target = loadTarget(parser, m_emulatorOption, m_deviceOption, true);

    if (target.isNull()) {
        return false;
    }

    // Whew. We made it! Let's get the controller now.
    Hemera::DeveloperMode::Controller::Ptr controller = retrieveDeveloperModeControllerOrDie(target);
    if (!controller) {
        return false;
    }

    QStringList args;
    QString interface;
    if (parser->isSet(m_interfaceOption)) {
        interface = parser->value(m_interfaceOption);
        args << QStringLiteral("-i") << interface;

    } else if (!m_applicationId.isEmpty()) {
        args << QStringLiteral("--orbital-application") << m_applicationId;
    }

    Hemera::DeveloperMode::ShellOperation *debuggerOperation = remoteExecute(QStringLiteral("/usr/libexec/hemera/tools/hemera-debug-helper"), args);
    connect(this, &GDBCommand::term, debuggerOperation, &Hemera::DeveloperMode::ShellOperation::interrupt);


    if (!m_applicationId.isEmpty()) {
        QStringList releaseArgs;
        releaseArgs << QStringLiteral("--orbital-application") << m_applicationId << QStringLiteral("--release-holder");
        Hemera::DeveloperMode::ShellOperation *operation = controller->executeShellCommand(QStringLiteral("/usr/libexec/hemera/tools/hemera-debug-helper"), releaseArgs);

        connect(operation, &Hemera::DeveloperMode::ShellOperation::stdoutMessage, [this] (const QByteArray &message) {
            std::cout.write(message.constData(), message.count());
            Q_EMIT stdoutMessage(message);
        });
        connect(operation, &Hemera::DeveloperMode::ShellOperation::stderrMessage, [this] (const QByteArray &message) {
            std::cerr.write(message.constData(), message.count());
        });
        connect(operation, &Hemera::DeveloperMode::ShellOperation::finished, [this, operation] {
            if (operation->isError()) {
                std::cout << tr("Error!").toStdString() << ' ' << operation->errorMessage().toStdString() << std::endl;
            }
        });
    }

    return true;
}

void GDBCommand::setupParser(QCommandLineParser* parser, const QStringList& arguments)
{
    Q_UNUSED(arguments);

    BaseRemoteCommand::setupParser(parser, arguments);
    parser->addOption(m_interfaceOption);
    parser->addPositionalArgument("application_id", "ID of the Orbital Application to execute.");
}

bool GDBCommand::onTermRequest()
{
    emit term();
    return true;
}
