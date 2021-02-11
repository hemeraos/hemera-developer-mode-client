#include "hemeratargetfactory.h"

#include "hemeraconstants.h"
#include "hemeradevice.h"
#include "hemerawizards.h"
#include "hemeraemulator.h"

#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodetargetmanager.h>

#include <coreplugin/icore.h>
#include <extensionsystem/pluginmanager.h>

#include <utils/qtcassert.h>

namespace Hemera {
namespace Internal {

HemeraTargetFactory::HemeraTargetFactory(QObject *parent)
    : ProjectExplorer::IDeviceFactory(parent)
{
    setObjectName(QStringLiteral("HemeraTargetFactory"));
}

HemeraTargetFactory::~HemeraTargetFactory()
{
}

QString HemeraTargetFactory::displayNameForId(Core::Id type) const
{
    if (type == Constants::HEMERA_DEVICE_TYPE_EMULATOR) {
        return tr("Hemera Emulator");
    } else if (type == Constants::HEMERA_DEVICE_TYPE_DEVICE) {
        return tr("Hemera Device");
    }

    return QString();
}

bool HemeraTargetFactory::canCreate() const
{
    return true;
}


QList<Core::Id> HemeraTargetFactory::availableCreationIds() const
{
    return QList<Core::Id>() << Core::Id(Constants::HEMERA_DEVICE_TYPE_EMULATOR)
                             << Core::Id(Constants::HEMERA_DEVICE_TYPE_DEVICE);
}

ProjectExplorer::IDevice::Ptr HemeraTargetFactory::create(Core::Id id) const
{
    if (id == Constants::HEMERA_DEVICE_TYPE_EMULATOR) {
        // Pop up our emulator wizard
        HemeraEmulatorConfigurationWizard wizard(Core::ICore::mainWindow());
        if (wizard.exec() != QDialog::Accepted) {
            return ProjectExplorer::IDevice::Ptr();
        }

        return wizard.emulator();
    } else if (id == Constants::HEMERA_DEVICE_TYPE_DEVICE) {
        // Pop up our device wizard
        qDebug() << "Factory is creating the device";
        HemeraDeviceConfigurationWizard wizard(Core::ICore::mainWindow());
        if (wizard.exec() != QDialog::Accepted) {
            qDebug() << "Device creation failed!";
            return ProjectExplorer::IDevice::Ptr();
        }

        ProjectExplorer::IDevice::Ptr device = wizard.device();
        qDebug() << "Factory has created the device successfully." << device->id().toString();
        return device;
    }

    return ProjectExplorer::IDevice::Ptr();
}

bool HemeraTargetFactory::canRestore(const QVariantMap &map) const
{
    if (ProjectExplorer::IDevice::typeFromMap(map) == Constants::HEMERA_DEVICE_TYPE_EMULATOR) {
        // Check if there is an emulator matching this map.
        return !DeveloperMode::TargetManager::emulatorNameFromQuery(ProjectExplorer::IDevice::idFromMap(map).toString()).isEmpty();
    } else if (ProjectExplorer::IDevice::typeFromMap(map) == Constants::HEMERA_DEVICE_TYPE_DEVICE) {
        // Check if there is a device matching this map.
        return !DeveloperMode::TargetManager::deviceNameFromQuery(ProjectExplorer::IDevice::idFromMap(map).toString()).isEmpty();
    }

    return false;
}

ProjectExplorer::IDevice::Ptr HemeraTargetFactory::restore(const QVariantMap &map) const
{
    qDebug() << "Restoring device!!";
    QTC_ASSERT(canRestore(map), return ProjectExplorer::IDevice::Ptr());
    if (ProjectExplorer::IDevice::typeFromMap(map) == Constants::HEMERA_DEVICE_TYPE_EMULATOR) {
        QString emulatorName = DeveloperMode::TargetManager::emulatorNameFromQuery(ProjectExplorer::IDevice::idFromMap(map).toString());
        Hemera::DeveloperMode::Emulator::Ptr nativeEmulator = DeveloperMode::TargetManager::instance()->loadEmulator(emulatorName);
        const ProjectExplorer::IDevice::Ptr emulator = HemeraEmulator::create(nativeEmulator);
        emulator->fromMap(map);
        return emulator;
    } else if (ProjectExplorer::IDevice::typeFromMap(map) == Constants::HEMERA_DEVICE_TYPE_DEVICE) {
        QString deviceName = DeveloperMode::TargetManager::deviceNameFromQuery(ProjectExplorer::IDevice::idFromMap(map).toString());
        Hemera::DeveloperMode::Device::Ptr nativeDevice = DeveloperMode::TargetManager::instance()->loadDevice(deviceName);
        const ProjectExplorer::IDevice::Ptr device = HemeraDevice::create(nativeDevice);
        device->fromMap(map);
        return device;
    }

    return ProjectExplorer::IDevice::Ptr();
}

HemeraTargetFactory *HemeraTargetFactory::instance()
{
    // Get factory
    QObject *factoryObj = ExtensionSystem::PluginManager::getObjectByClassName(QStringLiteral("Hemera::Internal::HemeraTargetFactory"));
    if (!factoryObj) {
        qWarning() << "Could not get target factory from plugin manager!!";
        return nullptr;
    }
    HemeraTargetFactory *factory = qobject_cast<HemeraTargetFactory*>(factoryObj);
    if (!factory) {
        qWarning() << "Could not get target factory from plugin manager released object!!";
        return nullptr;
    }

    return factory;
}

}
}
