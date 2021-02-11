#ifndef HEMERA_INTERNAL_HEMERAWIZARDS_H
#define HEMERA_INTERNAL_HEMERAWIZARDS_H

#include <utils/wizard.h>

#include <projectexplorer/devicesupport/idevice.h>

namespace Hemera {
namespace Internal {

class HemeraDeviceConfigurationWizard : public Utils::Wizard
{
    Q_OBJECT

public:
    explicit HemeraDeviceConfigurationWizard(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~HemeraDeviceConfigurationWizard();

    ProjectExplorer::IDevice::Ptr device();

    static bool checkHsdkInternal(QWidget *parent);

private Q_SLOTS:
    void checkHsdk();
};

class HemeraEmulatorConfigurationWizard : public Utils::Wizard
{
    Q_OBJECT

public:
    explicit HemeraEmulatorConfigurationWizard(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~HemeraEmulatorConfigurationWizard();

    ProjectExplorer::IDevice::Ptr emulator();

private Q_SLOTS:
    void checkHsdk();

};

} // namespace Internal
} // namespace Hemera

#endif // HEMERA_INTERNAL_HEMERAWIZARDS_H
