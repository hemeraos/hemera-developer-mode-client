#include "hsdkcommand.h"

#include "deploycommand.h"
#include "emulatorcommand.h"
#include "hacommand.h"
#include "invokecommand.h"
#include "launchcommand.h"
#include "outputcommand.h"
#include "gdbcommand.h"
#include "devicecommand.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <iostream>

HsdkCommand::HsdkCommand(const QStringList &arguments, QObject *parent)
    : BaseCommand(parent)
    , m_arguments(arguments)
    , m_dumpOption("dump", tr("Outputs version and SDK information in a machine-readable format."))
{
    addSubcommand(new BuildPackageCommand(this));
    addSubcommand(new BuildProjectCommand(this));
    addSubcommand(new CmakeCommand(this));
    addSubcommand(new ConfigureCommand(this));
    addSubcommand(new DeployCommand(this));
    addSubcommand(new EmulatorCommand(this));
    addSubcommand(new FullBuildCommand(this));
    addSubcommand(new HaCommand(this));
    addSubcommand(new InvokeCommand(this));
    addSubcommand(new LaunchCommand(this));
    addSubcommand(new OutputCommand(this));
    addSubcommand(new GDBCommand(this));
    addSubcommand(new WipeCommand(this));
    addSubcommand(new DeviceCommand(this));

    connect(this, &BaseCommand::finished, this, &HsdkCommand::onFinished);
}

QString HsdkCommand::briefDescription()
{
    return "The Hemera SDK command line tool.";
}

QString HsdkCommand::longDescription()
{
    return "The Hemera SDK command line tool.";
}

QString HsdkCommand::name()
{
    return "hsdk";
}

void HsdkCommand::onFinished(int exitStatus)
{
    QCoreApplication::exit(exitStatus);
}

bool HsdkCommand::parseAndExecute(QCommandLineParser *parser, const QStringList &arguments)
{
    Q_UNUSED(arguments)

    QStringList optionNames = parser->optionNames();

    if (parser->positionalArguments().isEmpty() && optionNames.size() == 1 && (optionNames.contains("v") || optionNames.contains("version"))) {
        QString versionString = qApp->applicationName() + " version " + qApp->applicationVersion();
        std::cout << versionString.toStdString() << std::endl;
        Q_EMIT finished(0);
        return true;
    } else if (parser->positionalArguments().isEmpty() && parser->isSet(m_dumpOption)) {
        // Dump machine readable information.
        QJsonObject info;

        QStringList supportedHemeraSDKs;
        supportedHemeraSDKs << QStringLiteral("Aegis");
        supportedHemeraSDKs << QStringLiteral("Next");

        info.insert(QStringLiteral("supportedHemeraSDKs"), QJsonArray::fromStringList(supportedHemeraSDKs));
        info.insert(QStringLiteral("executablePath"), QCoreApplication::applicationFilePath());
        info.insert(QStringLiteral("version"), qApp->applicationVersion());

        std::cout << QJsonDocument(info).toJson(QJsonDocument::Compact).constData() << std::endl;

        Q_EMIT finished(0);
        return true;
    }

    printErrorAndExit("No parameters. Use --help flag for usage info.");
    return false;
}

void HsdkCommand::setupParser(QCommandLineParser *parser, const QStringList &arguments)
{
    Q_UNUSED(arguments);
    parser->addVersionOption();
    parser->addOption(m_dumpOption);
}

void HsdkCommand::start()
{
    execute(m_arguments);
}
