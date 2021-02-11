#include "hacommand.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>

#include <hemeradevelopermodehamanager.h>

#include <iostream>

void printQmlErrors(const QList<QQmlError> &errors)
{
    for (const QQmlError &error : errors) {
        std::cout << error.toString().toStdString() << std::endl;
    }
}

HaCommand::HaCommand(QObject *parent)
    : BaseCommand(parent)
{
    addSubcommand(new InspectHaCommand(this));
    addSubcommand(new AddFileToHaCommand(this));
}

HaCommand::~HaCommand()
{
}

QString HaCommand::briefDescription()
{
    return QStringLiteral("Manage ha files.");
}

QString HaCommand::longDescription()
{
    return QStringLiteral("Manage ha files");
}

QString HaCommand::name()
{
    return QStringLiteral("ha");
}


//////////////////////////
InspectHaCommand::InspectHaCommand(QObject* parent)
    : BaseCommand(parent)
{
}

InspectHaCommand::~InspectHaCommand()
{
}

QString InspectHaCommand::briefDescription()
{
    return tr("Inspects a ha file.");
}

QString InspectHaCommand::longDescription()
{
    return tr("Inspects a ha file.");
}

QString InspectHaCommand::name()
{
    return QStringLiteral("inspect");
}

bool InspectHaCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    QStringList positionalArguments = parser->positionalArguments();

    QString fileName;

    if (positionalArguments.count() != 1) {
        QDir currentDir = QDir::current();
        for (const QFileInfo &entry : currentDir.entryInfoList(QStringList() << QStringLiteral("*.ha"))) {
            fileName = entry.absoluteFilePath();
            break;
        }

        if (fileName.isEmpty()) {
            std::cout << tr("No filename specified, and no ha found in current directory.").toStdString() << std::endl;
            Q_EMIT finished(1);
            return false;
        }
    } else {
        fileName = positionalArguments.at(0);
    }

    Hemera::DeveloperMode::HaManager *manager = new Hemera::DeveloperMode::HaManager(fileName, this);
    connect(manager, &Hemera::DeveloperMode::HaManager::readyChanged, [this, manager] (bool ready) {
        if (!ready) {
            return;
        }

        std::cout << tr("Ha file for %1 (%2), version %3.").arg(manager->applicationName(), manager->applicationId(),
                                                                manager->applicationVersion()).toStdString() << std::endl;
        std::cout << tr("Type: %1").arg(Hemera::DeveloperMode::HaManager::typeToString(manager->type())).toStdString() << std::endl;
        if (manager->isCompiledApplication()) {
            std::cout << tr("Source files: %1").arg(manager->fileList(Hemera::DeveloperMode::HaManager::FileCategory::SourceFiles).join(QStringLiteral(", "))).toStdString() << std::endl;
        }
        std::cout << tr("Resource files: %1").arg(manager->fileList(Hemera::DeveloperMode::HaManager::FileCategory::ResourceFiles).join(QStringLiteral(", "))).toStdString() << std::endl;
        Q_EMIT finished(0);
    });
    connect(manager, &Hemera::DeveloperMode::HaManager::qmlErrors, [this, manager] (const QList<QQmlError> &errors) {
        std::cout << tr("Parsing of the ha file failed!").toStdString() << std::endl;
        printQmlErrors(errors);
        Q_EMIT finished(1);
    });
    connect(manager, &Hemera::DeveloperMode::HaManager::qmlWarnings, [this, manager] (const QList<QQmlError> &errors) {
        std::cout << tr("Problems were found while parsing the ha file!").toStdString() << std::endl;
        printQmlErrors(errors);
        Q_EMIT finished(1);
    });

    return true;
}

void InspectHaCommand::setupParser(QCommandLineParser* parser, const QStringList& arguments)
{
    parser->addPositionalArgument("ha_file", "Ha file to inspect. If empty, it tries to find one in the current directory.");
}


//////////////////////////
AddFileToHaCommand::AddFileToHaCommand(QObject* parent)
    : BaseCommand(parent)
{
}

AddFileToHaCommand::~AddFileToHaCommand()
{
}

QString AddFileToHaCommand::briefDescription()
{
    return tr("Adds files to an ha file.");
}

QString AddFileToHaCommand::longDescription()
{
    return tr("Adds files to an ha file.");
}

QString AddFileToHaCommand::name()
{
    return QStringLiteral("add");
}

bool AddFileToHaCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    QStringList positionalArguments = parser->positionalArguments();

    QString fileName;

    QDir currentDir = QDir::current();
    for (const QFileInfo &entry : currentDir.entryInfoList(QStringList() << QStringLiteral("*.ha"))) {
        fileName = entry.absoluteFilePath();
        break;
    }

    if (fileName.isEmpty()) {
        std::cout << tr("No ha found in current directory.").toStdString() << std::endl;
        Q_EMIT finished(1);
        return false;
    }

    if (positionalArguments.count() < 1) {
        std::cout << tr("No files specified.").toStdString() << std::endl;
        Q_EMIT finished(1);
        return false;
    }

    QStringList files = positionalArguments;

    Hemera::DeveloperMode::HaManager *manager = new Hemera::DeveloperMode::HaManager(fileName, this);
    connect(manager, &Hemera::DeveloperMode::HaManager::readyChanged, [this, manager, files] (bool ready) {
        if (!ready) {
            return;
        }

        bool success = manager->addFiles(files, Hemera::DeveloperMode::HaManager::FileCategory::SourceFiles);
        if (!success) {
            std::cout << tr("Could not add files.").toStdString() << std::endl;
        }

        Q_EMIT finished(success ? 0 : 1);
    });
    connect(manager, &Hemera::DeveloperMode::HaManager::qmlErrors, [this, manager] (const QList<QQmlError> &errors) {
        std::cout << tr("Parsing of the ha file failed!").toStdString() << std::endl;
        printQmlErrors(errors);
        Q_EMIT finished(1);
    });
    connect(manager, &Hemera::DeveloperMode::HaManager::qmlWarnings, [this, manager] (const QList<QQmlError> &errors) {
        std::cout << tr("Problems were found while parsing the ha file!").toStdString() << std::endl;
        printQmlErrors(errors);
        Q_EMIT finished(1);
    });

    return true;
}

void AddFileToHaCommand::setupParser(QCommandLineParser* parser, const QStringList& arguments)
{
    parser->addPositionalArgument("files", "Files to add to the Ha file in the current directory.");
}
