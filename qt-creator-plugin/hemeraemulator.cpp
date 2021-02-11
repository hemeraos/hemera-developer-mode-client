#include "hemeraemulator.h"

#include "ui_hemeraemulatorconfigurationwidget.h"

#include <hemeradevelopermodeemulator.h>

#include "hemeraconstants.h"

namespace Hemera {
namespace Internal {

HemeraEmulator::HemeraEmulator(const Hemera::DeveloperMode::Emulator::Ptr &nativeEmulator)
    : HemeraTarget(nativeEmulator, Core::Id(Constants::HEMERA_DEVICE_TYPE_EMULATOR), Emulator)
{
}

HemeraEmulator::~HemeraEmulator()
{
}

HemeraEmulator::Ptr HemeraEmulator::create(const DeveloperMode::Emulator::Ptr &nativeEmulator)
{
    return Ptr(new HemeraEmulator(nativeEmulator));
}

QList<Core::Id> HemeraEmulator::actionIds() const
{
    QList<Core::Id> actions = HemeraTarget::actionIds();
    actions << Constants::START_STOP_EMULATOR_ACTION_ID;
    return actions;
}

QString HemeraEmulator::displayNameForActionId(Core::Id actionId) const
{
    if (actionId == Constants::START_STOP_EMULATOR_ACTION_ID) {
        DeveloperMode::Emulator::Ptr emulator = target().objectCast<DeveloperMode::Emulator>();
        if (!emulator->isRunning()) {
            return tr("Start Emulator");
        } else {
            return tr("Stop Emulator");
        }
    }

    return HemeraTarget::displayNameForActionId(actionId);
}

void HemeraEmulator::executeAction(Core::Id actionId, QWidget *parent)
{
    if (actionId == Constants::START_STOP_EMULATOR_ACTION_ID) {
        DeveloperMode::Emulator::Ptr emulator = target().objectCast<DeveloperMode::Emulator>();
        if (!emulator->isRunning()) {
            emulator->start();
        } else {
            emulator->stop();
        }
    } else {
        HemeraTarget::executeAction(actionId, parent);
    }
}

QString HemeraEmulator::displayType() const
{
    return tr("Hemera Emulator");
}

ProjectExplorer::IDeviceWidget *HemeraEmulator::createWidget()
{
    return new HemeraEmulatorConfigurationWidget(sharedFromThis(), target().objectCast<DeveloperMode::Emulator>());
}

ProjectExplorer::IDevice::Ptr HemeraEmulator::clone() const
{
    return Ptr(new HemeraEmulator(target().objectCast<DeveloperMode::Emulator>()));
}

QStringList HemeraEmulator::hsdkArguments() const
{
    QStringList arguments;
    arguments << QStringLiteral("-e") << target()->name();
    return arguments;
}



HemeraEmulatorConfigurationWidget::HemeraEmulatorConfigurationWidget(const ProjectExplorer::IDevice::Ptr &deviceConfig,
                                                                     const DeveloperMode::Emulator::Ptr &emulator, QWidget *parent)
    : ProjectExplorer::IDeviceWidget(deviceConfig, parent)
    , ui(new Ui::HemeraEmulatorConfigurationWidget)
    , m_emulator(emulator)
{
    ui->setupUi(this);

    QString trimmedId = emulator->applianceId();
    trimmedId = trimmedId.mid(0, 12);
    trimmedId.append(QStringLiteral("..."));
    ui->applianceIdLabel->setText(trimmedId);
    ui->emulatorIdLabel->setText(emulator->id());
    ui->buildForArchitectureLabel->setText(emulator->buildArchitectures().join(QStringLiteral(", ")));
    // Check for our emulator

}

HemeraEmulatorConfigurationWidget::~HemeraEmulatorConfigurationWidget()
{
    delete ui;
}

void HemeraEmulatorConfigurationWidget::updateDeviceFromUi()
{

}

} // namespace Internal
} // namespace Hemera

