#include "baseremotecommand.h"

#include <QtCore/QDebug>
#include <QtCore/QSocketNotifier>
#include <QtCore/QThread>

#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodetargetmanager.h>
#include <hemeradevelopermodeshelloperation.h>

#include <iostream>
#include <string>
#include <sstream>

BaseRemoteCommand::BaseRemoteCommand(QObject *parent)
    : BaseCommand(parent)
    , m_emulatorOption(QStringList() << "e" << "emulator", tr("Build for the emulator."), "emulator")
    , m_deviceOption(QStringList() << "d" << "device", tr("The Hemera device you want to build for."), "device")
{
}

bool BaseRemoteCommand::parseAndExecute(QCommandLineParser *parser, const QStringList &arguments)
{
    Q_UNUSED(arguments)

    if (parser->isSet(m_emulatorOption) && parser->isSet(m_deviceOption)) {
        printErrorAndExit("You can't use \"emulator\" and \"device\" options together. Use --help flag for usage info.");
        return false;
    }

    Hemera::DeveloperMode::Emulator::Ptr target = loadEmulatorEnvironment(parser, m_emulatorOption, m_deviceOption, true);

    if (target.isNull()) {
        return false;
    }

    m_controller = retrieveDeveloperModeControllerOrDie(target);
    if (m_controller.isNull()) {
        return false;
    }

    return true;
}

void BaseRemoteCommand::setupParser(QCommandLineParser *parser, const QStringList &arguments)
{
    Q_UNUSED(arguments)

    parser->addOption(m_deviceOption);
    parser->addOption(m_emulatorOption);
}

Hemera::DeveloperMode::ShellOperation *BaseRemoteCommand::remoteExecute(const QString &command, const QStringList &arguments, const QString &workingDirectory)
{
    Hemera::DeveloperMode::ShellOperation *operation;
    if (workingDirectory.isEmpty()) {
        operation = m_controller->executeShellCommand(command, arguments);
    } else {
        operation = m_controller->executeShellCommand(command, arguments, QStringList(), workingDirectory);
    }

    // Handle stdin
    QThread *thread = new QThread(this);
    QPointer<StdInReader> reader = new StdInReader;

    connect(thread, &QThread::started, reader.data(), &StdInReader::start);
    connect(reader.data(), &StdInReader::stdInData, [operation] (const QByteArray &data) {
        //FIXME: they should pass here a buffer that already contains \n
        QByteArray withNewLine = data;
        withNewLine.append('\n');
        //HACK: Ugly hack to avoid that any host system path gets leaked inside the VM
        withNewLine.replace(QDir::toNativeSeparators(QDir::homePath()), "/home/developer");
        operation->writeStdin(withNewLine);
    });
    connect(reader.data(), &StdInReader::stdInEOF, [operation] {
        operation->sendStdinEOF();
    });
    connect(reader.data(), &QObject::destroyed, thread, &QThread::quit);

    reader->moveToThread(thread);
    thread->start();

    // Automatic flushing after every output operation
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);

    connect(operation, &Hemera::DeveloperMode::ShellOperation::stdoutMessage, [this] (const QByteArray &message) {
        // message already contains new lines, message might be a multiline string
        std::cout.write(message.constData(), message.count());
#if 0
        // Work around cmake...
        if (message.endsWith("%]")) {
            std::cout << " ";
        }
#endif
        Q_EMIT stdoutMessage(message);
    });
    connect(operation, &Hemera::DeveloperMode::ShellOperation::stderrMessage, [this] (const QByteArray &message) {
        // message already contains new lines, message might be a multiline string
        std::cerr.write(message.constData(), message.count());
    });
    connect(operation, &Hemera::DeveloperMode::ShellOperation::finished, [this, operation, thread, reader] {
        if (operation->isError()) {
            std::cerr << tr("Error!").toStdString() << ' ' << operation->errorMessage().toStdString() << std::endl;
        }

        // Kill the thread
        if (!reader.isNull()) {
            delete reader;
        }
        thread->quit();

        Q_EMIT finished(operation->isError() ? 1 : 0);
    });

    return operation;
}



StdInReader::StdInReader(QObject* parent)
    : QObject(parent)
{
}

StdInReader::~StdInReader()
{

}

void StdInReader::start()
{
    for (std::string line; std::getline(std::cin, line);) {
        //FIXME
        Q_EMIT stdInData(QByteArray::fromRawData(line.data(), line.size()));
    }

    // Send EOF
    Q_EMIT stdInEOF();
    deleteLater();
}
