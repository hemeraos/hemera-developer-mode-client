#include "hemeradeployconfiguration.h"

#include "hemeraconstants.h"
#include "hemeradeploystep.h"
#include "hemerakitinformation.h"
#include "hemerasdkmanager.h"
#include "hemeratarget.h"
#include "hsdkstep.h"

#include <hemeradevelopermodetargetmanager.h>

#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/project.h>
#include <projectexplorer/target.h>
#include <projectexplorer/toolchain.h>

#include <qtsupport/qtkitinformation.h>
#include <qtsupport/qtsupportconstants.h>

namespace Hemera {
namespace Internal {

HemeraDeployConfiguration::HemeraDeployConfiguration(ProjectExplorer::Target *parent, Core::Id id)
    : DeployConfiguration(parent, id)
{
    setDisplayName(tr("Deploy to Hemera device or emulator"));
    setDefaultDisplayName(displayName());

    m_nativeTarget = DeveloperMode::TargetManager::instance()->loadTarget(HemeraKitInformation::targetName(parent->kit()));
}

HemeraDeployConfiguration::HemeraDeployConfiguration(ProjectExplorer::Target *parent, ProjectExplorer::DeployConfiguration *source)
    : DeployConfiguration(parent, source)
{
    cloneSteps(source);
    m_nativeTarget = DeveloperMode::TargetManager::instance()->loadTarget(HemeraKitInformation::targetName(parent->kit()));
}

DeveloperMode::Target::Ptr HemeraDeployConfiguration::nativeTarget() const
{
    return m_nativeTarget;
}

HemeraDeployConfigurationFactory::HemeraDeployConfigurationFactory(QObject *parent)
    : DeployConfigurationFactory(parent)
{
    setObjectName(QLatin1String("HemeraDeployConfigurationFactory"));
}

bool HemeraDeployConfigurationFactory::canCreate(ProjectExplorer::Target *parent, Core::Id id) const
{
    return availableCreationIds(parent).contains(id);
}

ProjectExplorer::DeployConfiguration *HemeraDeployConfigurationFactory::create(ProjectExplorer::Target *parent, Core::Id id)
{
    // Find out about our device
    ProjectExplorer::IDevice::ConstPtr idevice = ProjectExplorer::DeviceKitInformation::device(parent->kit());
    HemeraTarget::ConstPtr hemeraTarget = idevice.dynamicCast<const HemeraTarget>();
    if (hemeraTarget.isNull()) {
        qDebug() << "Oh fuck!!";
        return 0;
    }

    HemeraDeployConfiguration *dc = new HemeraDeployConfiguration(parent, id);

    dc->stepList()->insertStep(0, new HemeraDeployStep(dc->stepList()));

    return dc;
}

bool HemeraDeployConfigurationFactory::canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const
{
    return canCreate(parent, ProjectExplorer::idFromMap(map));
}

ProjectExplorer::DeployConfiguration *HemeraDeployConfigurationFactory::restore(ProjectExplorer::Target *parent, const QVariantMap &map)
{
    if (!canRestore(parent, map))
        return 0;

    HemeraDeployConfiguration *dc = new HemeraDeployConfiguration(parent, ProjectExplorer::idFromMap(map));
    if (dc->fromMap(map))
        return dc;

    delete dc;
    return 0;
}

bool HemeraDeployConfigurationFactory::canClone(ProjectExplorer::Target *parent, ProjectExplorer::DeployConfiguration *source) const
{
    if (!HemeraSDKManager::validateKit(parent->kit()))
        return false;
    return canCreate(parent, source->id());
}

ProjectExplorer::DeployConfiguration *HemeraDeployConfigurationFactory::clone(ProjectExplorer::Target *parent, ProjectExplorer::DeployConfiguration *source)
{
    if (!canClone(parent, source))
        return 0;
    return new HemeraDeployConfiguration(parent, source);
}

QList<Core::Id> HemeraDeployConfigurationFactory::availableCreationIds(ProjectExplorer::Target *parent) const
{
    QList<Core::Id> ids;
    if (!parent->project()->supportsKit(parent->kit())) {
        return ids;
    }

    if (!HemeraSDKManager::validateKit(parent->kit())) {
        return ids;
    }

    ids << Core::Id(Constants::DEPLOY_CONFIG_ID);
    return ids;
}

QString HemeraDeployConfigurationFactory::displayNameForId(Core::Id id) const
{
    if (id == Core::Id(Constants::DEPLOY_CONFIG_ID)) {
        return tr("Deploy to Hemera Device or Emulator");
    }
    return QString();
}

bool HemeraDeployConfigurationFactory::canHandle(ProjectExplorer::Target *parent) const
{
    return HemeraSDKManager::validateKit(parent->kit());
}

} // namespace Internal
} // namespace Hemera

