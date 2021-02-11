#include "hemeradeviceconfigurationwidget.h"
#include "ui_hemeradeviceconfigurationwidget.h"

#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodeemulator.h>

namespace Hemera {
namespace Internal {

HemeraDeviceConfigurationWidget::HemeraDeviceConfigurationWidget(const ProjectExplorer::IDevice::Ptr &deviceConfig, const DeveloperMode::Device::Ptr &device, QWidget *parent)
    : ProjectExplorer::IDeviceWidget(deviceConfig, parent)
    , ui(new Ui::HemeraDeviceConfigurationWidget)
    , m_device(device)
{
    ui->setupUi(this);

    // We assume the device is already up.
    QString trimmedId = device->applianceId();
    trimmedId = trimmedId.mid(0, 12);
    trimmedId.append(QStringLiteral("..."));
    ui->applianceIDLabel->setText(trimmedId);
    trimmedId = device->id();
    trimmedId = trimmedId.mid(0, 12);
    trimmedId.append(QStringLiteral("..."));
    ui->hardwareIDLabel->setText(trimmedId);
    if (device->associatedEmulator()) {
        ui->associatedEmulatorLabel->setText(device->associatedEmulator()->name());
    }

    int freqValue = device->cpuFrequency();
    QString freqStr;

    if (freqValue > (1024*1024)) {
        freqStr = QStringLiteral("%1.%2 GHz").arg(freqValue / (1024*1024)).arg(qRound((freqValue % (1024*1024)) / 10000.0));
    } else if (freqValue % 1024) {
        freqStr = QStringLiteral("%1 MHz").arg(freqValue / 1024);
    } else {
        freqStr = QStringLiteral("%1 KHz").arg(freqValue);
    }

    ui->CPULabel->setText(QStringLiteral("%1 x %2").arg(device->availableCores()).arg(freqStr));

    QString memStr;
    int memValue = device->totalMemory();
    if (memValue > (1024*1024*1024)) {
        memStr = QStringLiteral("%1.%2 GB").arg(memValue / (1024*1024*1024)).arg(qRound((memValue % (1024*1024*1024)) / 10000000.0));
    } else {
        memStr = QStringLiteral("%1.%2 MB").arg(memValue / (1024*1024)).arg(qRound((memValue % (1024*1024)) / 10000.0));
    }
    ui->RAMLabel->setText(memStr);
    ui->machineTypeLabel->setText(device->targetName());
}

HemeraDeviceConfigurationWidget::~HemeraDeviceConfigurationWidget()
{
    delete ui;
}

void HemeraDeviceConfigurationWidget::updateDeviceFromUi()
{

}

} // namespace Internal
} // namespace Hemera
