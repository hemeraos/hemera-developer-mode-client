#include "hemerasdkmanager.h"

#include "hemeraconstants.h"
#include "hemeradevice.h"
#include "hemeraemulator.h"
#include "hemerakitinformation.h"
#include "hemeratoolchain.h"
#include "hemeraqtversion.h"
#include "hemeratarget.h"
#include "hemeratargetfactory.h"

#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodetargetmanager.h>

#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/qtcassert.h>
#include <utils/persistentsettings.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/toolchainmanager.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/devicesupport/devicemanager.h>
#include <qtsupport/qtversionmanager.h>
#include <qtsupport/qtkitinformation.h>
#include <ssh/sshkeygenerator.h>

#include <QtCore/QDir>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QMenu>

using namespace ProjectExplorer;

namespace Hemera {
namespace Internal {

HemeraSDKManager *HemeraSDKManager::m_instance = 0;
bool HemeraSDKManager::verbose = false;

static Utils::FileName globalSettingsFileName()
{
    QSettings *globalSettings = ExtensionSystem::PluginManager::globalSettings();
    return Utils::FileName::fromString(QFileInfo(globalSettings->fileName()).absolutePath()
                                       + QLatin1String(Constants::HEMERA_SDK_FILENAME));
}

static Utils::FileName settingsFileName()
{
     QFileInfo settingsLocation(ExtensionSystem::PluginManager::settings()->fileName());
     return Utils::FileName::fromString(settingsLocation.absolutePath() + QLatin1String(Constants::HEMERA_SDK_FILENAME));
}

HemeraSDKManager::HemeraSDKManager()
    : m_intialized(false),
      m_writer(0),
      m_reinstall(false)
{
    connect(KitManager::instance(), SIGNAL(kitsLoaded()), SLOT(initialize()));
    connect(DeviceManager::instance(), SIGNAL(devicesLoaded()), SLOT(updateDevices()));
    connect(ProjectExplorer::KitManager::instance(), &ProjectExplorer::KitManager::kitsLoaded, this, &HemeraSDKManager::updateDevices);
    connect(DeviceManager::instance(), &DeviceManager::devicesLoaded, [this] {
        // Connect other interesting signals now.
        connect(DeviceManager::instance(), SIGNAL(updated()), SLOT(updateDevices()));
        connect(DeveloperMode::TargetManager::instance(), &DeveloperMode::TargetManager::updated, this, &HemeraSDKManager::updateDevices);
    });
    // Creator quite sucks, so we need to hook up to the device removal this way.
    connect(DeviceManager::instance(), SIGNAL(deviceRemoved(Core::Id)), SLOT(onDeviceRemoved(Core::Id)));

    m_writer = new Utils::PersistentSettingsWriter(settingsFileName(), QLatin1String("HemeraSDKs"));
    m_instance = this;
    ProjectExplorer::KitManager::registerKitInformation(new HemeraKitInformation);
}

HemeraSDKManager::~HemeraSDKManager()
{
    m_instance = 0;
}

void HemeraSDKManager::initialize()
{
    if (!m_intialized) {
        restore();
        //read kits
        QList<Kit*> kits = hemeraKits();
        QList<HemeraToolChain*> toolchains = hemeraToolChains();
        QList<HemeraQtVersion*> qtversions = hemeraQtVersions();
        //cleanup
        foreach (HemeraToolChain *toolchain, toolchains) {
            QString targetName = toolchain->targetName();
            if (!DeveloperMode::TargetManager::availableDevices().contains(targetName) && !DeveloperMode::TargetManager::registeredEmulators().contains(targetName)) {
                qWarning() << "HemeraToolChain has invalid target associated. Removing toolchain.";
                ToolChainManager::deregisterToolChain(toolchain);
                continue;
            }
        }

        foreach (HemeraQtVersion *version, qtversions) {
            QString targetName = version->targetName();
            if (!DeveloperMode::TargetManager::availableDevices().contains(targetName) && !DeveloperMode::TargetManager::registeredEmulators().contains(targetName)) {
                qWarning() << "HemeraQtVersion has invalid target associated. Removing toolchain.";
                QtSupport::QtVersionManager::removeVersion(version);
                continue;
            }
        }

        //remove broken kits
        foreach (Kit *kit, kits) {
            if (!validateKit(kit)) {
                qWarning() << "Broken Hemera kit found! Removing kit.";
                KitManager::deregisterKit(kit);
            } else {
                kit->validate();
            }
        }

        m_intialized = true;
        emit initialized();
    }
}

QList<Kit *> HemeraSDKManager::hemeraKits() const
{
    QList<Kit*> kits;
    foreach (Kit *kit, KitManager::kits()) {
        if (isHemeraKit(kit))
            kits << kit;
    }
    return kits;
}

QList<HemeraToolChain *> HemeraSDKManager::hemeraToolChains() const
{
    QList<HemeraToolChain*> toolchains;
    foreach (ToolChain *toolchain, ToolChainManager::toolChains()) {
        if (!toolchain->isAutoDetected())
            continue;
        if (toolchain->type() != QLatin1String(Constants::TOOLCHAIN_TYPE))
            continue;
        toolchains << static_cast<HemeraToolChain*>(toolchain);
    }
    return toolchains;
}

QList<HemeraQtVersion *> HemeraSDKManager::hemeraQtVersions() const
{
    QList<HemeraQtVersion*> qtversions;
    foreach (QtSupport::BaseQtVersion *qtVersion, QtSupport::QtVersionManager::versions()) {
        if (!qtVersion->isAutodetected())
            continue;
        if (qtVersion->type() != QLatin1String(Constants::HEMERA_QT))
            continue;
        qtversions << static_cast<HemeraQtVersion*>(qtVersion);
    }
    return qtversions;
}

HemeraSDKManager *HemeraSDKManager::instance()
{
    QTC_CHECK(m_instance);
    return m_instance;
}

const Utils::FileName& HemeraSDKManager::checkInstallLocation(const Utils::FileName &local,
                                                              const Utils::FileName &global)
{
    Utils::PersistentSettingsReader lReader;
    if (!lReader.load(local)) {
        // local file not found
        return global;
    }

    Utils::PersistentSettingsReader gReader;
    if (!gReader.load(global)) {
        // global file read failed, use the local file then.
        return local;
    }

    QVariantMap lData = lReader.restoreValues();
    QVariantMap gData = gReader.restoreValues();

    QString lInstallDir = lData.value(QLatin1String(Constants::HEMERA_SDK_INSTALLDIR)).toString();
    QString gInstallDir = gData.value(QLatin1String(Constants::HEMERA_SDK_INSTALLDIR)).toString();

    // if the installdirectory has changed, return the global file
    if (lInstallDir != gInstallDir) {
        if (HemeraSDKManager::verbose)
            qDebug() << "HemeraSDKManager::installdir changed => use global config";
        m_reinstall = true;
        return global;
    }
    else {
        if (HemeraSDKManager::verbose)
            qDebug() << "HemeraSDKManager::installdir same => use local config";
        return local;
    }
}

void HemeraSDKManager::restore()
{
    checkInstallLocation(settingsFileName(), globalSettingsFileName());

    if (m_reinstall) {
        // This is executed if the user has reinstalled HemeraSDK to
        // a different directory. Clean up all the existing Hemera
        // kits, which contain paths to the old install directory.
        foreach (ProjectExplorer::Kit *kit, ProjectExplorer::KitManager::kits()) {
            if (!kit->isAutoDetected()) {
                continue;
            }
            ProjectExplorer::ToolChain *tc = ProjectExplorer::ToolChainKitInformation::toolChain(kit);
            if (!tc){
                continue;
            }

            if (tc->type() == QLatin1String(Constants::TOOLCHAIN_TYPE)) {
                if (HemeraSDKManager::verbose) {
                    qDebug() << "Removing Hemera kit due to reinstall";
                }
                QtSupport::BaseQtVersion *v = QtSupport::QtKitInformation::qtVersion(kit);
                ProjectExplorer::KitManager::deregisterKit(kit);
                ProjectExplorer::ToolChainManager::deregisterToolChain(tc);
                QtSupport::QtVersionManager::removeVersion(v);
            }
        }
    }
}

bool HemeraSDKManager::isHemeraKit(const Kit *kit)
{
    if (!kit) {
        return false;
    }

    ToolChain* tc = ToolChainKitInformation::toolChain(kit);
    const Core::Id deviceType = DeviceTypeKitInformation::deviceTypeId(kit);
    if (tc && tc->type() == QLatin1String(Constants::TOOLCHAIN_TYPE)) {
        return true;
    }

    if (deviceType.isValid() && HemeraTargetFactory::instance()->availableCreationIds().contains(deviceType))
        return true;

    return false;
}

QString HemeraSDKManager::targetNameForKit(const Kit *kit)
{
    if (!kit || !isHemeraKit(kit))
        return QString();
    ToolChain *toolchain = ToolChainKitInformation::toolChain(kit);
    if (toolchain && toolchain->type() == QLatin1String(Constants::TOOLCHAIN_TYPE)) {
        HemeraToolChain *hemeraToolChain = static_cast<HemeraToolChain *>(toolchain);
        return hemeraToolChain->targetName();
    }
    return QString();
}

QList<Kit *> HemeraSDKManager::kitsForTarget(const QString &targetName)
{
    QList<Kit*> kitsForTarget;
    if (targetName.isEmpty())
        return kitsForTarget;
    const QList<Kit*> kits = KitManager::kits();
    foreach (Kit *kit, kits) {
        if (targetNameForKit(kit) == targetName)
            kitsForTarget << kit;
    }
    return kitsForTarget;
}

bool HemeraSDKManager::hasHemeraDevice(Kit *kit)
{
    IDevice::ConstPtr dev = DeviceKitInformation::device(kit);
    if (dev.isNull())
        return false;
    return HemeraTargetFactory::instance()->availableCreationIds().contains(dev->type());
}

QString HemeraSDKManager::sdkToolsDirectory()
{
    return QFileInfo(ExtensionSystem::PluginManager::settings()->fileName()).absolutePath() +
            QLatin1String(Constants::HEMERA_SDK_TOOLS);
}

QString HemeraSDKManager::globalSdkToolsDirectory()
{
    return QFileInfo(ExtensionSystem::PluginManager::globalSettings()->fileName()).absolutePath() +
            QLatin1String(Constants::HEMERA_SDK_TOOLS);
}

bool HemeraSDKManager::validateKit(const Kit *kit)
{
    if (!kit) {
        return false;
    }
    ToolChain* toolchain = ToolChainKitInformation::toolChain(kit);
    QtSupport::BaseQtVersion* version = QtSupport::QtKitInformation::qtVersion(kit);
    Core::Id deviceType = DeviceTypeKitInformation::deviceTypeId(kit);

    if (!version || !toolchain || !deviceType.isValid()) {
        return false;
    }
    if (version->type() != QLatin1String(Constants::HEMERA_QT)) {
        return false;
    }
    if (toolchain->type() != QLatin1String(Constants::TOOLCHAIN_TYPE)) {
        return false;
    }
    if (!HemeraTargetFactory::instance()->availableCreationIds().contains(deviceType)) {
        return false;
    }
    if (version->platformName() != QLatin1String(Constants::HEMERA_QT_PLATFORM)) {
        return false;
    }

    HemeraToolChain* hemeraToolChain = static_cast<HemeraToolChain*>(toolchain);
    HemeraQtVersion* hemeraQtVersion = static_cast<HemeraQtVersion*>(version);

    return hemeraToolChain->targetName() == hemeraQtVersion->targetName();
}

void HemeraSDKManager::updateDevices()
{
    // We need all of our systems up
    if (!ProjectExplorer::DeviceManager::instance()->isLoaded() || !QtSupport::QtVersionManager::isLoaded()) {
        return;
    }

    // In a similar fashion to what we do with Kits, we need to investigate what happens here and remove devices, just in case.
    Hemera::DeveloperMode::TargetManager *targetManager = Hemera::DeveloperMode::TargetManager::instance();
    ProjectExplorer::DeviceManager *deviceManager = ProjectExplorer::DeviceManager::instance();

    QStringList availableNativeDevices = targetManager->availableDevices();
    QStringList availableNativeEmulators = targetManager->registeredEmulators().keys();

    qDebug() << "Updating devices!! Currently device manager has this many devices: " << deviceManager->deviceCount();

    // Ok, Creator tries as hard as possible to make our life hard. We need to proactively update our cache to make
    // sure we can intercept user removal from the device model, and act accordingly.
    QList<Core::Id> localCachedIds;

    for (int i = 0; i < deviceManager->deviceCount(); ++i) {
        ProjectExplorer::IDevice::ConstPtr device = deviceManager->deviceAt(i);
        if (device->type() != Constants::HEMERA_DEVICE_TYPE_DEVICE && device->type() != Constants::HEMERA_DEVICE_TYPE_EMULATOR) {
            // Not our business.
            continue;
        }

        if (device->type() == Constants::HEMERA_DEVICE_TYPE_DEVICE) {
            if (availableNativeDevices.contains(device->displayName())) {
                availableNativeDevices.removeOne(device->displayName());
            } else {
                // We have to remove this device
                qDebug() << "Removing" << device->displayName() << "as it's no longer registered in Hemera";
                deviceManager->removeDevice(device->id());
                continue;
            }
        } else if (device->type() == Constants::HEMERA_DEVICE_TYPE_EMULATOR) {
            if (availableNativeEmulators.contains(device->displayName())) {
                availableNativeEmulators.removeOne(device->displayName());
            } else {
                // We have to remove this device
                qDebug() << "Removing" << device->displayName() << "as it's no longer registered in Hemera";
                deviceManager->removeDevice(device->id());
                continue;
            }
        }

        localCachedIds << device->id();

        // Do we already have this in our cache? If so, skip
        if (m_hemeraDevicesCache.contains(device->id())) {
            continue;
        }

        // Regardless, we need to register our kits.
        qDebug() << "SDK Manager is registering for the first time target" << device->displayName();
        HemeraTarget::ConstPtr hemeraTarget = device.dynamicCast<const HemeraTarget>();
        hemeraTarget->registerTargetKits();
        m_hemeraDevicesCache.insert(hemeraTarget->id(), hemeraTarget);
    }

    // Did we intercept any user removal?
    QHash<Core::Id, HemeraTarget::ConstPtr>::iterator i = m_hemeraDevicesCache.begin();
    while (i != m_hemeraDevicesCache.end()) {
        if (localCachedIds.contains(i.key())) {
            ++i;
        } else {
            // We have to remove it.
            bool result = false;
            if (i.value()->machineType() == ProjectExplorer::IDevice::Emulator) {
                // Stop the emulator, if running
                if (DeveloperMode::TargetManager::instance()->runningEmulator()->id() == i.value()->target()->id()) {
                    DeveloperMode::TargetManager::instance()->runningEmulator()->stop()->synchronize(5000);
                }

                result = DeveloperMode::TargetManager::instance()->removeEmulator(i.value()->id().toString(), false);
                availableNativeEmulators.removeOne(i.value()->id().toString());
            } else {
                result = DeveloperMode::TargetManager::instance()->removeKnownDevice(i.value()->id().toString());
                availableNativeDevices.removeOne(i.value()->id().toString());
            }

            if (!result) {
                qWarning() << "Could not remove native Hemera target!!";
            }

            i = m_hemeraDevicesCache.erase(i);
        }
    }

    // Ok. Now, is there anything left?
    for (const QString &deviceName : availableNativeDevices) {
        qDebug() << "SDK manager is adding missing device!" << deviceName;
        deviceManager->addDevice(HemeraDevice::create(targetManager->loadDevice(deviceName)));
    }

    for (const QString &emulatorName : availableNativeEmulators) {
        qDebug() << "SDK manager is adding missing emulator!" << emulatorName;
        deviceManager->addDevice(HemeraEmulator::create(targetManager->loadEmulator(emulatorName)));
    }
}

void HemeraSDKManager::onDeviceRemoved(const Core::Id &id)
{
    qDebug() << "Device removed, " << id.toString();
}

} // namespace Internal
} // namespace Hemera

