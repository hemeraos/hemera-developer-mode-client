#include "devicecommand.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTimer>

#include <hemeradevelopermodetarget.h>
#include <hemeradevelopermodecontroller.h>
#include <hemeradevelopermodedeployoperation.h>

#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodetargetmanager.h>
#include <hemeradevelopermodehyperdiscoveryclient.h>

#include <iostream>

DeviceCommand::DeviceCommand(QObject *parent)
    : BaseCommand(parent)
{
    addSubcommand(new AddDeviceCommand(this));
    addSubcommand(new RemoveDeviceCommand(this));
    addSubcommand(new ScanDeviceCommand(this));
    addSubcommand(new ListDeviceCommand(this));
    addSubcommand(new AssociateDeviceCommand(this));
}

DeviceCommand::~DeviceCommand()
{
}

QString DeviceCommand::briefDescription()
{
    return QStringLiteral("Manage Hemera devices.");
}

QString DeviceCommand::longDescription()
{
    return QStringLiteral("Manage devices");
}

QString DeviceCommand::name()
{
    return QStringLiteral("device");
}


//////////////////////////
AddDeviceCommand::AddDeviceCommand(QObject* parent)
    : BaseCommand(parent)
{
}

AddDeviceCommand::~AddDeviceCommand()
{
}

QString AddDeviceCommand::briefDescription()
{
    return tr("Adds a known or static device.");
}

QString AddDeviceCommand::longDescription()
{
    return tr("Adds a known or static device.");
}

QString AddDeviceCommand::name()
{
    return QStringLiteral("add");
}

bool AddDeviceCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    QStringList positionalArguments = parser->positionalArguments();

    if (positionalArguments.count() != 2) {
        std::cout << "Usage: targetName targetPath" << std::endl;
        Q_EMIT finished(1);
        return false;
    }

    QString targetName = positionalArguments.at(0);
    QString targetPath = positionalArguments.at(1);

    Hemera::DeveloperMode::TargetManager *tm = Hemera::DeveloperMode::TargetManager::instance();

    if (tm->availableDevices().contains(targetName)) {
        std::cerr << tr("Error: There is already a target with the name \"%1\".").arg(targetName).toStdString() << std::endl;
        Q_EMIT finished(1);
        return true;
    } else {
        QUrl targetUrl(targetPath);
        if (!targetUrl.scheme().isEmpty()) {
            Hemera::DeveloperMode::TargetManager::instance()->createStaticDevice(targetName, targetUrl, QCoreApplication::applicationFilePath());
        } else {
            Hemera::DeveloperMode::TargetManager::instance()->createKnownDevice(targetName, targetPath, QCoreApplication::applicationFilePath());
        }
    }

    Hemera::DeveloperMode::Target::Ptr target = tm->loadTarget(targetName);

    if (target.isNull()) {
        qWarning() << "Error: there is no target named " << targetName;
        Q_EMIT finished(1);
        return true;
    }

    Hemera::DeveloperMode::IsAuthRequiredOperation *authorizedOperation = target->isAuthRequired(QStringLiteral("com.ispirata.Hemera.DeveloperMode/"));
    bool status = authorizedOperation->synchronize(3000);
    if (!status) {
        qWarning() << "We don't know if auth is required or not: " << authorizedOperation->result();
        Q_EMIT finished(1);
        return true;
    }
    if (!authorizedOperation->result()) {
        Q_EMIT finished(0);
        return true;
    }

    Hemera::DeveloperMode::NetworkByteArrayOperation *pairingO = target->startAuth(QStringLiteral("com.ispirata.Hemera.DeveloperModeAuth"),
                                                   QStringLiteral("com.ispirata.Hemera.DeviceInfo"),
                                                   QStringLiteral("/"), QStringLiteral("*"), 20);
    status = pairingO->synchronize(3000);

    if (status) {
        if (pairingO->status() != 200) {
            qWarning() << "Pairing Error: " << pairingO->result();

        } else {
            QByteArray pairingToken = pairingO->result();

            std::cout << tr("Insert device PIN: " ).toStdString();
            QTextStream stream(stdin);
            QString pin = stream.readLine();
            Hemera::DeveloperMode::NetworkByteArrayOperation *pairing1 = target->sendAuthPIN(QStringLiteral("com.ispirata.Hemera.DeveloperModeAuth"), pairingToken, pin);
            status = pairing1->synchronize(3000);
            if (!status) {
                qWarning() << "Pairing Error: " << pairingO->result();
            } else {
                std::cout << tr("Device successfully paired.").toStdString() << std::endl;
                Q_EMIT finished(0);
                return true;
            }
        }

    } else {
        qDebug() << "Pairing timeout";
    }

    return true;
}

void AddDeviceCommand::setupParser(QCommandLineParser* parser, const QStringList& arguments)
{
    parser->addPositionalArgument("target_name", "User defined target name.");
    parser->addPositionalArgument("target_url_or_id", "Device URL or id.");
}


RemoveDeviceCommand::RemoveDeviceCommand(QObject* parent)
    : BaseCommand(parent)
{
}

RemoveDeviceCommand::~RemoveDeviceCommand()
{
}

QString RemoveDeviceCommand::briefDescription()
{
    return tr("Removes a known or static device.");
}

QString RemoveDeviceCommand::longDescription()
{
    return tr("Removes a known or static device.");
}

QString RemoveDeviceCommand::name()
{
    return QStringLiteral("remove");
}

bool RemoveDeviceCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    QString targetName = parser->positionalArguments().at(0);
    bool success = Hemera::DeveloperMode::TargetManager::instance()->removeKnownDevice(targetName);
    Q_EMIT finished(success ? 0 : 1);

    return success;
}

void RemoveDeviceCommand::setupParser(QCommandLineParser* parser, const QStringList& arguments)
{
    parser->addPositionalArgument("target_name", "User defined target name.");
}


ScanDeviceCommand::ScanDeviceCommand(QObject* parent)
    : BaseCommand(parent)
    , m_timeoutTimer(new QTimer(this))
{
    // Default timeout is 3 seconds
    m_timeoutTimer->setInterval(3000);
    m_timeoutTimer->setSingleShot(true);

    connect(m_timeoutTimer, &QTimer::timeout, [this] {
        Q_EMIT finished(1);
    });
}

ScanDeviceCommand::~ScanDeviceCommand()
{
}

QString ScanDeviceCommand::briefDescription()
{
    return tr("Removes a known or static device.");
}

QString ScanDeviceCommand::longDescription()
{
    return tr("Removes a known or static device.");
}

QString ScanDeviceCommand::name()
{
    return QStringLiteral("scan");
}

bool ScanDeviceCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    Hemera::DeveloperMode::HyperDiscoveryClient *hDiscovery = new Hemera::DeveloperMode::HyperDiscoveryClient();

    connect(hDiscovery, &Hemera::DeveloperMode::HyperDiscoveryClient::capabilityDiscovered, [this] (QByteArray capability, const QHostAddress &address, int port, int ttl) {
        QString hardwareId = QString::fromLatin1(capability);
        hardwareId.remove(QStringLiteral("hwid."));
        QString deviceName = Hemera::DeveloperMode::TargetManager::deviceNameFromQuery(hardwareId);
        if (deviceName.isEmpty()) {
            deviceName = tr("Unregistered");
        }
        std::cout << QStringLiteral("|%1 (%2)\t|%3\t|%4\t|").arg(deviceName, hardwareId, address.toString()).arg(port).toStdString() << std::endl;
    });

    m_timeoutTimer->start();
    hDiscovery->scanCapabilities(QList<QByteArray>() << "hwid.*");
}



ListDeviceCommand::ListDeviceCommand(QObject* parent)
    : BaseCommand(parent)
{
}

ListDeviceCommand::~ListDeviceCommand()
{
}

QString ListDeviceCommand::briefDescription()
{
    return tr("Removes a known or static device.");
}

QString ListDeviceCommand::longDescription()
{
    return tr("Removes a known or static device.");
}

QString ListDeviceCommand::name()
{
    return QStringLiteral("list");
}

bool ListDeviceCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    Hemera::DeveloperMode::TargetManager *tm = Hemera::DeveloperMode::TargetManager::instance();

    for (const QString &targetName : tm->availableDevices()) {
        std::cout << targetName.toStdString() << std::endl;
    }

    Q_EMIT finished(0);
}


AssociateDeviceCommand::AssociateDeviceCommand(QObject* parent)
    : BaseCommand(parent)
{
}

AssociateDeviceCommand::~AssociateDeviceCommand()
{
}

QString AssociateDeviceCommand::briefDescription()
{
    return tr("Removes a known or static device.");
}

QString AssociateDeviceCommand::longDescription()
{
    return tr("Removes a known or static device.");
}

QString AssociateDeviceCommand::name()
{
    return QStringLiteral("associate");
}

bool AssociateDeviceCommand::parseAndExecute(QCommandLineParser* parser, const QStringList& arguments)
{
    QStringList positionalArguments = parser->positionalArguments();

    if (positionalArguments.count() != 2) {
        printErrorAndExit("Usage: deviceName emulatorName");
        return false;
    }

    QString deviceName = positionalArguments.at(0);
    QString emulatorName = positionalArguments.at(1);

    Hemera::DeveloperMode::Device::Ptr device = retrieveDeviceOrDie(deviceName);
    Hemera::DeveloperMode::Emulator::Ptr emulator = retrieveEmulatorOrDie(emulatorName);

    if (!device) {
        printErrorAndExit(tr("Device %1 does not exist!").arg(deviceName));
        return false;
    }

    if (!emulator) {
        printErrorAndExit(tr("Emulator %1 does not exist!").arg(emulatorName));
        return false;
    }

    if (Hemera::DeveloperMode::TargetManager::instance()->associate(device, emulator)) {
        std::cout << tr("Device successfully associated.").toStdString() << std::endl;
        Q_EMIT finished(0);
        return true;
    }

    printErrorAndExit(tr("Could not associate device!"));
    return false;
}

void AssociateDeviceCommand::setupParser(QCommandLineParser* parser, const QStringList& arguments)
{
    parser->addPositionalArgument("device_name", tr("Device to be associated."));
    parser->addPositionalArgument("emulator_name", tr("Emulator to be associated to the device."));
}
