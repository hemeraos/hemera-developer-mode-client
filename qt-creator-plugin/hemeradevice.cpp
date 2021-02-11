#include "hemeradevice.h"

#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodeemulator.h>

#include "hemeradeviceconfigurationwidget.h"
#include "hemeraconstants.h"

namespace Hemera {
namespace Internal {

HemeraDevice::HemeraDevice(const Hemera::DeveloperMode::Device::Ptr &nativeDevice)
    : HemeraTarget(nativeDevice, Core::Id(Constants::HEMERA_DEVICE_TYPE_DEVICE), Hardware)
{
}

HemeraDevice::~HemeraDevice()
{
}

HemeraDevice::Ptr HemeraDevice::create(const DeveloperMode::Device::Ptr &nativeDevice)
{
    return Ptr(new HemeraDevice(nativeDevice));
}

QString HemeraDevice::displayType() const
{
    return tr("Hemera Device");
}

ProjectExplorer::IDeviceWidget *HemeraDevice::createWidget()
{
    return new HemeraDeviceConfigurationWidget(sharedFromThis(), target().objectCast<DeveloperMode::Device>());
}

QList<Core::Id> HemeraDevice::actionIds() const
{
    return HemeraTarget::actionIds();
}

QString HemeraDevice::displayNameForActionId(Core::Id actionId) const
{
    return HemeraTarget::displayNameForActionId(actionId);
}

void HemeraDevice::executeAction(Core::Id actionId, QWidget *parent)
{
    HemeraTarget::executeAction(actionId, parent);
}

ProjectExplorer::IDevice::Ptr HemeraDevice::clone() const
{
    return Ptr(new HemeraDevice(target().objectCast<DeveloperMode::Device>()));
}

Core::Id HemeraDevice::associatedEmulator()
{
    DeveloperMode::Device::Ptr device = target().objectCast<DeveloperMode::Device>();
    return Core::Id::fromString(device->associatedEmulator()->name());
}

QStringList HemeraDevice::hsdkArguments() const
{
    QStringList arguments;
    arguments << QStringLiteral("-d") << target()->name();
    return arguments;
}

} // namespace Internal
} // namespace Hemera

