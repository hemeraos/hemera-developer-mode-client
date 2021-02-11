#ifndef HEMERA_INTERNAL_TARGET_H
#define HEMERA_INTERNAL_TARGET_H

#include <QtCore/QCoreApplication>

#include <projectexplorer/devicesupport/idevice.h>

#include <hemeradevelopermodetarget.h>

namespace Hemera {
namespace DeveloperMode {
class Target;
}
namespace Internal {

class HemeraQtVersion;
class HemeraToolChain;

class HemeraTarget : public ProjectExplorer::IDevice
{
    Q_DECLARE_TR_FUNCTIONS(Hemera::Internal::HemeraTarget)
public:
    virtual ~HemeraTarget();

    typedef QSharedPointer<HemeraTarget> Ptr;
    typedef QSharedPointer<const HemeraTarget> ConstPtr;

    virtual bool isCompatibleWith(const ProjectExplorer::Kit *k) const override;

    virtual QString displayType() const override;
    virtual ProjectExplorer::IDeviceWidget *createWidget() override;
    virtual QList<Core::Id> actionIds() const override;
    virtual QString displayNameForActionId(Core::Id actionId) const override;
    virtual void executeAction(Core::Id actionId, QWidget *parent = 0) override;

    // Devices that can auto detect ports need not return a ports gathering method. Such devices can
    // obtain a free port on demand. eg: Desktop device.
    virtual bool canAutoDetectPorts() const override final { return false; }
    virtual ProjectExplorer::PortsGatheringMethod::Ptr portsGatheringMethod() const override final { return ProjectExplorer::IDevice::portsGatheringMethod(); }
    virtual bool canCreateProcessModel() const override final { return true; }
    virtual ProjectExplorer::DeviceProcessList *createProcessListModel(QObject *parent = 0) const override final;
    virtual bool hasDeviceTester() const override final { return true; }
    virtual ProjectExplorer::DeviceTester *createDeviceTester() const override final;

    virtual bool canCreateProcess() const override final { return true; }
    virtual ProjectExplorer::DeviceProcess *createProcess(QObject *parent) const override final;
    virtual ProjectExplorer::DeviceProcessSignalOperation::Ptr signalOperation() const override final;

    virtual ProjectExplorer::IDevice::Ptr clone() const override;

    virtual QString qmlProfilerHost() const override;


    // This is our own stuff!!
    ProjectExplorer::Kit* createKit() const;
    HemeraQtVersion* createQtVersion() const;
    HemeraToolChain* createToolChain() const;

    void registerTargetKits() const;

    virtual QStringList hsdkArguments() const = 0;

    Hemera::DeveloperMode::Target::Ptr target() const;

protected:
    explicit HemeraTarget(const DeveloperMode::Target::Ptr &target, Core::Id type,
                          MachineType machineType);

private:
    class Private;
    Private * const d;

    friend class HemeraDeviceTester;
    friend class HemeraRunControl;
    friend class HemeraProject;
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERA_INTERNAL_TARGET_H
