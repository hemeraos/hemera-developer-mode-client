#include "hemeradevicetester.h"

#include "hemeratarget.h"

#include <hemeradevelopermodecontroller.h>
#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodetargetmanager.h>

#include <QtCore/QTimer>

namespace Hemera {
namespace Internal {

HemeraDeviceTester::HemeraDeviceTester(QObject *parent)
    : ProjectExplorer::DeviceTester(parent)
{
}

HemeraDeviceTester::~HemeraDeviceTester()
{
}

void HemeraDeviceTester::testDevice(const ProjectExplorer::IDevice::ConstPtr &deviceConfiguration)
{
    // It needs to be a HemeraTarget
    const HemeraTarget::ConstPtr target = qSharedPointerDynamicCast<const HemeraTarget>(deviceConfiguration);
    QString id = deviceConfiguration->id().toString();

    emit progressMessage(tr("Verifying target..."));

    if (target.isNull()) {
        qDebug() << "Target not found!";
        emit errorMessage(tr("Device with id %1 was not found!").arg(id));
        Q_EMIT finished(ProjectExplorer::DeviceTester::TestFailure);
    }

    // Is it an emulator?
    if (target->machineType() == ProjectExplorer::IDevice::Emulator) {
        // Turn it on
        DeveloperMode::Target::Ptr nativeTarget = target->target();
        DeveloperMode::Emulator::Ptr nativeEmulator = nativeTarget.objectCast<DeveloperMode::Emulator>();

        if (nativeEmulator.isNull()) {
            emit errorMessage(tr("Could not acquire native target for Emulator!"));
            emit finished(ProjectExplorer::DeviceTester::TestFailure);
            return;
        }

        // Is it running?
        if (!nativeEmulator->isRunning()) {
            emit progressMessage(tr("Starting emulator..."));
            connect (nativeEmulator->start(), &DeveloperMode::Operation::finished, [this, target] (DeveloperMode::Operation *op) {
                if (op->isError()) {
                    emit errorMessage(tr("Could not start Emulator! %1 - %2").arg(op->errorName(), op->errorMessage()));
                    emit finished(ProjectExplorer::DeviceTester::TestFailure);
                    return;
                }

                testDeveloperModeController(target);
            });
            return;
        }
    }

    testDeveloperModeController(target);
}

void HemeraDeviceTester::testDeveloperModeController(const HemeraTarget::ConstPtr &target)
{
    emit progressMessage(tr("Acquiring Developer Mode Controller lock..."));

    connect(target->target()->ensureDeveloperModeController(30000), &DeveloperMode::Operation::finished, this,
            [this, target] (DeveloperMode::Operation *operation) {
        if (operation->isError()) {
            emit errorMessage(tr("Developer Mode Controller could not be retrieved: %1 - %2").arg(operation->errorName(), operation->errorMessage()));
            if (!target->target()->isOnline()) {
                emit progressMessage(tr("Target appears to be offline!"));
                Q_EMIT finished(ProjectExplorer::DeviceTester::TestFailure);
            } else {
                emit progressMessage(tr("Could not retrieve Developer Mode Controller for Target!"));
                Q_EMIT finished(ProjectExplorer::DeviceTester::TestFailure);
            }
        } else {
            Q_EMIT finished(ProjectExplorer::DeviceTester::TestSuccess);
        }
    });
}

void HemeraDeviceTester::stopTest()
{
    // There's no actual way to stop the test, so just simulate a timeout.
    emit errorMessage(tr("Test aborted!"));
    Q_EMIT finished(ProjectExplorer::DeviceTester::TestFailure);
}

} // namespace Internal
} // namespace Hemera

