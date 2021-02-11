#include "deploycommand.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTimer>

#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodetargetmanager.h>
#include <hemeradevelopermodecontroller.h>
#include <hemeradevelopermodedeployoperation.h>

#include <iostream>

DeployCommand::DeployCommand(QObject *parent)
    : BaseCommand(parent)
    , m_emulatorOption(QStringList() << "e" << "emulator", "Build for the emulator.", "emulator")
    , m_deviceOption(QStringList() << "d" << "device", "The Hemera device you want to build for.", "device")
    , m_timeoutTimer(new QTimer(this))
{
    // Default timeout is 3 seconds.
    m_timeoutTimer->setInterval(3000);
    m_timeoutTimer->setSingleShot(true);

    connect(m_timeoutTimer, &QTimer::timeout, [this] { printErrorAndExit(tr("Operation timed out, exiting.")); });
}

QString DeployCommand::briefDescription()
{
    return "Deploy and install a RPM on a Hemera target.";
}

QString DeployCommand::longDescription()
{
    return "Deploy and install a RPM on a Hemera target.\n"
           "You can define targets using the Hemera Developer Mode Client or directly\n"
           "in the file '~/.config/Hemera/targets.ini' with this syntax:\n"
           "    [TARGET]\n"
           "    host=<IP OR HOSTNAME>\n"
           "    port=<SSH PORT>\n"
           "    username=<USERNAME>\n"
           "    password=<PASSWORD>\n"
           "    architecture=<ARCHITECTURE>\n"
           "Hemera SDK virtual machine default target is:\n"
           "    [vm]\n"
           "    host=127.0.0.1\n"
           "    port=2223\n"
           "    username=root\n"
           "    password=rootme\n"
           "    architecture=i586\n"
           "Hemera for Raspberry Pi default target is:\n"
           "    [rpi]\n"
           "    host=<IP OF YOUR RASPBERRY PI>\n"
           "    port=22\n"
           "    username=root\n"
           "    password=rootme\n"
           "    architecture=armv6l";
}

QString DeployCommand::name()
{
    return "deploy";
}

bool DeployCommand::parseAndExecute(QCommandLineParser *parser, const QStringList &arguments)
{
    Q_UNUSED(arguments)

    QStringList positionalArguments = parser->positionalArguments();
    if (positionalArguments.size() < 1) {
        printErrorAndExit("Missing required parameters. Use --help flag for usage info.");
        return false;
    }
    if (positionalArguments.size() > 1) {
        printErrorAndExit("Too many parameters. Use --help flag for usage info.");
        return false;
    }
    if (parser->isSet(m_emulatorOption) && parser->isSet(m_deviceOption)) {
        printErrorAndExit("You can't use \"emulator\" and \"device\" options together. Use --help flag for usage info.");
        return false;
    }

    QString rpmPath = positionalArguments.at(0);
    QFileInfo rpmInfo(rpmPath);
    m_rpmName = rpmInfo.absoluteFilePath();

    Hemera::DeveloperMode::Target::Ptr target = loadTarget(parser, m_emulatorOption, m_deviceOption, true);

    if (target.isNull()) {
        return false;
    }

    m_controller = retrieveDeveloperModeControllerOrDie(target);
    if (m_controller.isNull()) {
        return false;
    }

    // FIXME HACK WTF: Without this, either the server or the client gets the same fd and goes totally nuts.
    QEventLoop e;
    QTimer::singleShot(200, &e, SLOT(quit()));
    e.exec();

    // Deploy now.
    Hemera::DeveloperMode::DeployOperation *operation = m_controller->deployPackage(m_rpmName);
    connect(operation, &Hemera::DeveloperMode::Operation::finished, [this, operation] {
        if (operation->isError()) {
            std::cout << " error!" << std::endl;
            std::cerr << "Error occurred while deploying package: " << operation->errorName().toStdString()
                      << " - " << operation->errorMessage().toStdString() << std::endl;
            Q_EMIT finished(1);
        } else {
            std::cout << " success!" << std::endl << "Package deployed successfully" << std::endl;
            Q_EMIT finished(0);
        }
    });
    connect(operation, &Hemera::DeveloperMode::DeployOperation::progress, [] (quint64 bytesUploaded, quint64 totalBytes, quint64 rate) {
        std::cout << "\rUploading to target... " << (bytesUploaded / totalBytes) * 100 << "%";
        if (rate > 0) {
            std::cout << ", " << rate / 1024 << " KB/s";
        }
        std::cout.flush();

        if (bytesUploaded == totalBytes) {
            // Install phase started
            std::cout << std::endl << "Installing application to target...";
            std::cout.flush();
        }
    });

    return true;
}

void DeployCommand::setupParser(QCommandLineParser *parser, const QStringList &arguments)
{
    Q_UNUSED(arguments)

    parser->addOption(m_deviceOption);
    parser->addOption(m_emulatorOption);
    parser->addPositionalArgument("rpm_path", "Path of the RPM file to install.");
}
