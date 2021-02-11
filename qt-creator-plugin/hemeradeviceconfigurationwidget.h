#ifndef HEMERA_INTERNAL_HEMERADEVICECONFIGURATIONWIDGET_H
#define HEMERA_INTERNAL_HEMERADEVICECONFIGURATIONWIDGET_H

#include <projectexplorer/devicesupport/idevicewidget.h>

#include <hemeradevelopermodedevice.h>

namespace Hemera {
namespace Internal {

namespace Ui {
class HemeraDeviceConfigurationWidget;
}

class HemeraDeviceConfigurationWidget : public ProjectExplorer::IDeviceWidget
{
    Q_OBJECT

public:
    explicit HemeraDeviceConfigurationWidget(const ProjectExplorer::IDevice::Ptr &deviceConfig, const DeveloperMode::Device::Ptr &device, QWidget *parent = 0);
    virtual ~HemeraDeviceConfigurationWidget();

    virtual void updateDeviceFromUi() override final;

private:
    Ui::HemeraDeviceConfigurationWidget *ui;

    Hemera::DeveloperMode::Device::Ptr m_device;
};


} // namespace Internal
} // namespace Hemera
#endif // HEMERA_INTERNAL_HEMERADEVICECONFIGURATIONWIDGET_H
