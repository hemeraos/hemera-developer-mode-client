#include "remotebuildcommand.h"

#include <QtCore/QDebug>

#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodetarget.h>
#include <hemeradevelopermodetargetmanager.h>
#include <hemeradevelopermodecontroller.h>

RemoteBuildCommand::RemoteBuildCommand(QObject *parent)
    : BaseRemoteCommand(parent)
{
}

QString RemoteBuildCommand::extraArguments(QCommandLineParser* parser, const QStringList& arguments)
{
    Q_UNUSED(parser);
    Q_UNUSED(arguments);
    return QString();
}

bool RemoteBuildCommand::parseAndExecute(QCommandLineParser *parser, const QStringList &arguments)
{
    if (!BaseRemoteCommand::parseAndExecute(parser, arguments)) {
        return false;
    }

    QString archString;

    if (parser->isSet(m_emulatorOption)) {
        archString = QStringLiteral("i586");
    } else if (parser->isSet(m_deviceOption)) {
        QString deviceName = parser->value(m_deviceOption);
        Hemera::DeveloperMode::Device::Ptr device = retrieveDeviceOrDie(deviceName, false);
        if (!device) {
            return false;
        }
        archString = device->architecture();
    } else {
        // Defaults to emulator.
        archString = QStringLiteral("i586");
    }

    if (archString.isEmpty()) {
        printErrorAndExit("Empty architecture.");
        return false;
    }

    QString vmPathString = Hemera::DeveloperMode::Controller::localPathToVm();
    QStringList args = QStringList() << vmPathString << archString;

    // Add positional arguments as well.
    if (!parser->positionalArguments().isEmpty()) {
        args << parser->positionalArguments();
    }

    remoteExecute(remoteBuildCommand(), args);
    return true;
}

void RemoteBuildCommand::setupParser(QCommandLineParser *parser, const QStringList &arguments)
{
    BaseRemoteCommand::setupParser(parser, arguments);
}
