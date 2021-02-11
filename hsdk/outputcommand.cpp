#include "outputcommand.h"

#include <hemeradevelopermodeapplicationoutput.h>
#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodetargetmanager.h>

#include <QtCore/QDebug>
#include <QtCore/QJsonObject>
#include <QtCore/QTimer>

#include <iostream>

OutputCommand::OutputCommand(QObject *parent)
    : BaseCommand(parent)
    , m_emulatorOption(QStringList() << "e" << "emulator", tr("Choose a specific emulator"), "emulator")
    , m_deviceOption(QStringList() << "d" << "device", tr("Choose a specific Hemera Device."), "device")
{
}

QString OutputCommand::briefDescription()
{
    return tr("Print output of a running Hemera application.");
}

QString OutputCommand::longDescription()
{
    return tr("Print output of a running Hemera application.");
}

QString OutputCommand::name()
{
    return "output";
}

bool OutputCommand::parseAndExecute(QCommandLineParser *parser, const QStringList &arguments)
{
    QStringList positionalArguments = parser->positionalArguments();
    if (positionalArguments.size() < 1) {
        printErrorAndExit("Missing required parameters. Use --help flag for usage info.");
        return false;
    }
    if (positionalArguments.size() > 1) {
        printErrorAndExit("Too much parameters. Use --help flag for usage info.");
        return false;
    }

    QString applicationId = positionalArguments.at(0);

    // Load the target
    Hemera::DeveloperMode::Target::Ptr target = loadTarget(parser, m_emulatorOption, m_deviceOption, true);

    if (target.isNull()) {
        return false;
    }

    // FIXME HACK WTF: Without this, either the server or the client gets the same fd and goes totally nuts.
    QEventLoop e;
    QTimer::singleShot(200, &e, SLOT(quit()));
    e.exec();

    // Application output!
    Hemera::DeveloperMode::ApplicationOutput *applicationOutput = new Hemera::DeveloperMode::ApplicationOutput(target, applicationId, this);
    connect(applicationOutput, &Hemera::DeveloperMode::ApplicationOutput::newMessage, [this] (const QDateTime &timestamp, const QJsonObject &message) {
        std::cout << "[" << timestamp.toString().toStdString() << "] " << message.value(QStringLiteral("message")).toString().toStdString() << std::endl;
    });

    return true;
}

void OutputCommand::setupParser(QCommandLineParser* parser, const QStringList& arguments)
{
    Q_UNUSED(arguments);

    parser->addOption(m_deviceOption);
    parser->addOption(m_emulatorOption);
    parser->addPositionalArgument("application_id", tr("ID of the Application to monitor."));
}
