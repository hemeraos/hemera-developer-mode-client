#include "hemerakitinformation.h"

#include "hemeraconstants.h"
#include "hemeratarget.h"
#include "hemeratargetfactory.h"
#include "hemeratoolchain.h"

#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodetargetmanager.h>

#include <projectexplorer/projectexplorerconstants.h>

#include <utils/algorithm.h>
#include <utils/buildablehelperlibrary.h>
#include <utils/macroexpander.h>
#include <utils/qtcassert.h>

namespace Hemera {
namespace Internal {

HemeraKitInformation::HemeraKitInformation()
{
    setId(Constants::HEMERA_KIT_INFORMATION);
    setPriority(42);
}

HemeraKitInformation::~HemeraKitInformation()
{
}

QVariant HemeraKitInformation::defaultValue(ProjectExplorer::Kit *kit) const
{
    return HemeraKitInformation::targetName(kit);
}

QList<ProjectExplorer::Task> HemeraKitInformation::validate(const ProjectExplorer::Kit *kit) const
{
    if (HemeraTargetFactory::instance()->availableCreationIds().contains(ProjectExplorer::DeviceTypeKitInformation::deviceTypeId(kit))) {
        const QString &target = kit->value(Core::Id(Constants::HEMERA_TARGET_NAME)).toString();
        // Attempt to load the target. Is it a device?
        DeveloperMode::Device::Ptr device = DeveloperMode::TargetManager::instance()->loadDevice(target);
        if (!device.isNull()) {
            // Ok. Does the associated emulator exist?
            if (DeveloperMode::TargetManager::instance()->loadEmulatorForDevice(device).isNull()) {
                const QString message = tr("Hemera Device %1 has no valid associated Emulator.").arg(target);
                return QList<ProjectExplorer::Task>() << ProjectExplorer::Task(ProjectExplorer::Task::Error, message, Utils::FileName(), -1,
                                                                               Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM));
            }
        }
        // Maybe an emulator? (In this case it is self-sufficient, tho.
        DeveloperMode::Emulator::Ptr emulator = DeveloperMode::TargetManager::instance()->loadEmulator(target);
        if (device.isNull() && emulator.isNull()) {
            const QString message = tr("Target %1 referred by this kit does not exist.").arg(target);
            return QList<ProjectExplorer::Task>() << ProjectExplorer::Task(ProjectExplorer::Task::Error, message, Utils::FileName(), -1,
                                                                           Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM));
        }

        QList<ProjectExplorer::Task> errors;

        // Ok. Now we need to check for the consistency of the toolchains/devices/etc.
        ProjectExplorer::ToolChain *toolchain = ProjectExplorer::ToolChainKitInformation::toolChain(kit);
        HemeraToolChain *ht = dynamic_cast<HemeraToolChain*>(toolchain);
        if (!ht) {
            errors.append(ProjectExplorer::Task(ProjectExplorer::Task::Error, tr("A Hemera Kit needs to be paired with an Hemera toolchain."), Utils::FileName(), -1,
                                                Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_COMPILE)));
        } else {
            if (ht->targetName() != target) {
                const QString message = tr("A Hemera Kit needs its associated Toolchain. Please select the toolchain for %1 instead of %2.").arg(target, ht->targetName());
                errors.append(ProjectExplorer::Task(ProjectExplorer::Task::Error, message, Utils::FileName(), -1,
                                                    Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_COMPILE)));
            }
        }

        // And the device, too.
        ProjectExplorer::IDevice::ConstPtr idevice = ProjectExplorer::DeviceKitInformation::device(kit);
        HemeraTarget::ConstPtr hemeraTarget = idevice.dynamicCast<const HemeraTarget>();
        if (hemeraTarget.isNull()) {
            errors.append(ProjectExplorer::Task(ProjectExplorer::Task::Error, tr("A Hemera Kit needs to be paired with an Hemera Device or Emulator."),
                                                Utils::FileName(), -1, Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_COMPILE)));
        } else {
            if (target != hemeraTarget->displayName()) {
                const QString message = tr("A Hemera Kit needs its associated Device or Emulator. Please select the paired device.");
                errors.append(ProjectExplorer::Task(ProjectExplorer::Task::Error, message, Utils::FileName(), -1,
                                                    Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_COMPILE)));
            }
        }

        return errors;
    }

    return QList<ProjectExplorer::Task>();
}

QString HemeraKitInformation::targetName(const ProjectExplorer::Kit *kit)
{
    if (!kit)
        return QString();
    return kit->value(Core::Id(Constants::HEMERA_TARGET_NAME)).toString();
}

ProjectExplorer::KitInformation::ItemList HemeraKitInformation::toUserOutput(const ProjectExplorer::Kit *kit) const
{
    if (HemeraTargetFactory::instance()->availableCreationIds().contains(ProjectExplorer::DeviceTypeKitInformation::deviceTypeId(kit))) {
        QString targetName = HemeraKitInformation::targetName(kit);
        return ProjectExplorer::KitInformation::ItemList()
                << qMakePair(tr("HemeraTarget"),targetName);
    }
    return ProjectExplorer::KitInformation::ItemList();
}

ProjectExplorer::KitConfigWidget *HemeraKitInformation::createConfigWidget(ProjectExplorer::Kit *kit) const
{
    return Q_NULLPTR;
}

void HemeraKitInformation::setTargetName(ProjectExplorer::Kit *kit, const QString& targetName)
{
    if (kit->value(Core::Id(Constants::HEMERA_TARGET_NAME)) != targetName) {
        kit->setValue(Core::Id(Constants::HEMERA_TARGET_NAME),targetName);
    }
}

void HemeraKitInformation::addToEnvironment(const ProjectExplorer::Kit *kit, Utils::Environment &env) const
{
    const QString targetName = HemeraKitInformation::targetName(kit);
    env.appendOrSet(QLatin1String(Constants::HEMERA_TARGET_NAME),targetName);
}

QSet<QString> HemeraKitInformation::availablePlatforms(const ProjectExplorer::Kit *k) const
{
    const QString &targetName = HemeraKitInformation::targetName(k);
    if (targetName.isEmpty()) {
        return KitInformation::availablePlatforms(k);
    }

    QSet<QString> platforms;
    platforms << QLatin1String(Constants::HEMERA_QT_PLATFORM);
    return platforms;
}

QString HemeraKitInformation::displayNameForPlatform(const ProjectExplorer::Kit *k, const QString &platform) const
{
    if (platform == QLatin1String(Constants::HEMERA_QT_PLATFORM)) {
        return QLatin1String(Constants::HEMERA_QT_PLATFORM_TR);
    }

    return KitInformation::displayNameForPlatform(k, platform);
}

Core::FeatureSet HemeraKitInformation::availableFeatures(const ProjectExplorer::Kit *k) const
{
    const QString &targetName = HemeraKitInformation::targetName(k);
    if (targetName.isEmpty()) {
        return KitInformation::availableFeatures(k);
    }

    Core::FeatureSet features = KitInformation::availableFeatures(k);
    features |= Core::FeatureSet(Constants::FEATURE_HEMERA);
    return features;
}

ProjectExplorer::KitMatcher HemeraKitInformation::hemeraKitMatcher()
{
    return std::function<bool(const ProjectExplorer::Kit *)>([](const ProjectExplorer::Kit *kit) -> bool {
        if (HemeraKitInformation::targetName(kit).isEmpty()) {
            return false;
        }

        return true;
    });
}

} // namespace Internal
} // namespace Hemera

