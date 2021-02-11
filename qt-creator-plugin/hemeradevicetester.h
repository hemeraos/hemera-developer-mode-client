#ifndef HEMERA_INTERNAL_HEMERADEVICETESTER_H
#define HEMERA_INTERNAL_HEMERADEVICETESTER_H

#include <QtCore/QObject>

#include <hemeratarget.h>

#include <projectexplorer/devicesupport/idevice.h>

class QTimer;

namespace Hemera {
namespace Internal {

class HemeraDeviceTester : public ProjectExplorer::DeviceTester
{
    Q_OBJECT

public:
    explicit HemeraDeviceTester(QObject *parent = 0);
    virtual ~HemeraDeviceTester();

    virtual void testDevice(const ProjectExplorer::IDevice::ConstPtr &deviceConfiguration) override;
    virtual void stopTest() override;

private Q_SLOTS:
    void testDeveloperModeController(const HemeraTarget::ConstPtr &target);
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERA_INTERNAL_HEMERADEVICETESTER_H
