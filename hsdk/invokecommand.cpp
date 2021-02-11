#include "invokecommand.h"
#include <hemeradevelopermodetargetmanager.h>

#include <QtCore/QDebug>

#include <iostream>

const char ARGUMENT_QUERY[] = "-query";
const char ARGUMENT_DUMPMACHINE[] = "-dumpmachine";
const char ARGUMENT_DUMPSTUFFQTC[] = "-xc++ -E -dM -";

const char* cachedCommands[] =
{
    ARGUMENT_QUERY,
    ARGUMENT_DUMPMACHINE,
    ARGUMENT_DUMPSTUFFQTC
};

QHash<const char*, QString> argumentsToCommands = {
    std::pair<const char*, QString>(ARGUMENT_QUERY, QStringLiteral("qmake -query")),
    std::pair<const char*, QString>(ARGUMENT_DUMPMACHINE, QStringLiteral("gcc -dumpmachine")),
    std::pair<const char*, QString>(ARGUMENT_DUMPSTUFFQTC, QStringLiteral("gcc -xc++ -E -dM -"))
};

InvokeCommand::InvokeCommand(QObject* parent)
    : RemoteBuildCommand(parent)
    , m_failOnOffline(QStringList() << "fail-on-offline", tr("Fail immediately if the target is offline."))
{
}

InvokeCommand::~InvokeCommand()
{
}

QString InvokeCommand::longDescription()
{
    return tr("Invokes a command in a Hemera build environment.");
}

QString InvokeCommand::briefDescription()
{
    return tr("Invokes a command in a Hemera build environment.");
}

QString InvokeCommand::name()
{
    return QStringLiteral("invoke");
}

QString InvokeCommand::remoteBuildCommand()
{
    return QStringLiteral("hemera-invoke");
}

void InvokeCommand::setupParser(QCommandLineParser* parser, const QStringList& arguments)
{
    RemoteBuildCommand::setupParser(parser, arguments);

    parser->addOption(m_failOnOffline);
}

bool InvokeCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    // We need to check whether we're dealing with cached stuff or nah.
    bool couldBeCached = false;
    QString invokeCommand;
    for (const QString &argument : arguments) {
        for (const char *command : cachedCommands) {
            if (argument.endsWith(command)) {
                couldBeCached = true;
                invokeCommand = argumentsToCommands.value(command);
                break;
            }
        }
    }
    if (couldBeCached) {
        // It might as well be cached! Retrieve target (disconnected, of course).
        Hemera::DeveloperMode::Emulator::Ptr emulator = loadEmulatorEnvironment(parser, m_emulatorOption, m_deviceOption, false);
        QSettings *settings = Hemera::DeveloperMode::TargetManager::settingsForTarget(QStringLiteral("Emulators"), emulator->name());
        settings->beginGroup(QStringLiteral("cachedCommands")); {
            if (settings->childKeys().contains(arguments.last())) {
                // Cache power!!
                std::cout << settings->value(arguments.last()).toString().toStdString();
                delete settings;
                Q_EMIT finished(0);
                return true;
            }
        } settings->endGroup();

        // Didn't really work. So we want to cache the result then.
        connect(this, &RemoteBuildCommand::stdoutMessage, [this] (const QByteArray &message) {
            m_cachedOutput.append(message);
        });
        connect(this, &RemoteBuildCommand::finished, [this, settings, arguments] (int exitStatus) {
            if (exitStatus == 0) {
                // Save the cache!
                settings->beginGroup(QStringLiteral("cachedCommands")); {
                    settings->setValue(arguments.last(), m_cachedOutput);
                } settings->endGroup();
                settings->sync();
            }

            delete settings;
        });
    }

    if (parser->isSet(m_failOnOffline)) {
        // We have to fail (fast) if it is offline.
        Hemera::DeveloperMode::Emulator::Ptr target = loadEmulatorEnvironment(parser, m_emulatorOption, m_deviceOption, false);
        if (!target->ensureOnline(3000)->synchronize()) {
            std::cerr << tr("Target offline, failing as requested.").toStdString() << std::endl;
            Q_EMIT finished(157);
            return false;
        }
    }

    return RemoteBuildCommand::parseAndExecute(parser, arguments);
}
