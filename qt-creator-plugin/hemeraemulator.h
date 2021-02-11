#ifndef HEMERA_INTERNAL_HEMERAEMULATOR_H
#define HEMERA_INTERNAL_HEMERAEMULATOR_H

#include "hemeratarget.h"

#include <hemeradevelopermodeemulator.h>

#include <projectexplorer/devicesupport/idevicewidget.h>

namespace Hemera {
namespace Internal {
namespace Ui {
class HemeraEmulatorConfigurationWidget;
}

class HemeraEmulator : public Hemera::Internal::HemeraTarget
{
    Q_DECLARE_TR_FUNCTIONS(Hemera::Internal::HemeraEmulator)
public:
    explicit HemeraEmulator(const Hemera::DeveloperMode::Emulator::Ptr &nativeEmulator);
    virtual ~HemeraEmulator();

    typedef QSharedPointer<HemeraEmulator> Ptr;
    typedef QSharedPointer<const HemeraEmulator> ConstPtr;

    // To allow start and stop.
    virtual QList<Core::Id> actionIds() const override final;
    virtual QString displayNameForActionId(Core::Id actionId) const override final;
    virtual void executeAction(Core::Id actionId, QWidget *parent = 0) override final;

    static Ptr create(const DeveloperMode::Emulator::Ptr &nativeEmulator);

    virtual QString displayType() const override final;
    virtual ProjectExplorer::IDeviceWidget *createWidget() override final;

    virtual ProjectExplorer::IDevice::Ptr clone() const override final;

    virtual QStringList hsdkArguments() const override;
};

class HemeraEmulatorConfigurationWidget : public ProjectExplorer::IDeviceWidget
{
    Q_OBJECT

public:
    explicit HemeraEmulatorConfigurationWidget(const ProjectExplorer::IDevice::Ptr &deviceConfig, const DeveloperMode::Emulator::Ptr &emulator, QWidget *parent = 0);
    virtual ~HemeraEmulatorConfigurationWidget();

    virtual void updateDeviceFromUi() override final;

private:
    Ui::HemeraEmulatorConfigurationWidget *ui;

    Hemera::DeveloperMode::Emulator::Ptr m_emulator;
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERA_INTERNAL_HEMERAEMULATOR_H
