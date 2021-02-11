#ifndef HEMERA_INTERNAL_HEMERADEVICE_H
#define HEMERA_INTERNAL_HEMERADEVICE_H

#include "hemeratarget.h"

#include <hemeradevelopermodedevice.h>

namespace Hemera {
namespace DeveloperMode {
class Device;
}
namespace Internal {

class HemeraDevice : public Hemera::Internal::HemeraTarget
{
    Q_DECLARE_TR_FUNCTIONS(Hemera::Internal::HemeraDevice)
public:
    explicit HemeraDevice(const Hemera::DeveloperMode::Device::Ptr &nativeDevice);
    virtual ~HemeraDevice();

    typedef QSharedPointer<HemeraDevice> Ptr;
    typedef QSharedPointer<const HemeraDevice> ConstPtr;

    static Ptr create(const Hemera::DeveloperMode::Device::Ptr &nativeDevice);

    virtual QString displayType() const override final;
    virtual ProjectExplorer::IDeviceWidget *createWidget() override final;
    virtual QList<Core::Id> actionIds() const override final;
    virtual QString displayNameForActionId(Core::Id actionId) const override final;
    virtual void executeAction(Core::Id actionId, QWidget *parent = 0) override final;

    virtual ProjectExplorer::IDevice::Ptr clone() const override;

    Core::Id associatedEmulator();

    virtual QStringList hsdkArguments() const override;
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERA_INTERNAL_HEMERADEVICE_H
