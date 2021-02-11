#include "hemerawizards.h"

#include <hemeradevelopermodetargetmanager.h>

#include "hemeradevice.h"
#include "hemeraemulator.h"
#include "hemeraconfigurationpages.h"
#include "hemeraprojectmanager.h"

#include <QtCore/QTimer>
#include <QtWidgets/QMessageBox>

#include <extensionsystem/pluginmanager.h>

namespace Hemera {
namespace Internal {

bool HemeraDeviceConfigurationWizard::checkHsdkInternal(QWidget *parent)
{
    // Before we begin. We actually need to provide the path to hsdk, which has to be properly configured.
    HemeraSettingsPage *settingsPage = ExtensionSystem::PluginManager::getObject<HemeraSettingsPage>();
    if (!settingsPage) {
        QMessageBox::warning(parent, tr("Could not retrieve settings"), tr("Could not retrieve Hemera settings - this is an internal error."));
        return false;
    }

    QString pathToHsdk = settingsPage->hsdkExecutable();

    if (pathToHsdk.isEmpty()) {
        QMessageBox::warning(parent, tr("Hsdk not configured"), tr("Before adding a target, you need to configure Qt Creator to find the hsdk executable you want "
                                                                 "to use. You can do this in Tools->Options->Build & Run->Hemera."));
        return false;
    }

    return true;
}

HemeraDeviceConfigurationWizard::HemeraDeviceConfigurationWizard(QWidget *parent, Qt::WindowFlags flags)
    : Utils::Wizard(parent, flags)
{
    setWindowTitle(tr("New Hemera Device Setup"));

    // Before we begin. We actually need to provide the path to hsdk, which has to be properly configured.
    HemeraSettingsPage *settingsPage = ExtensionSystem::PluginManager::getObject<HemeraSettingsPage>();
    if (!settingsPage) {
        return;
    }

    QString pathToHsdk = settingsPage->hsdkExecutable();

    addPage(new HemeraDeviceConfigureWizardPage(this));
    addPage(new HemeraDeviceAssociateWizardPage(this));
    HemeraDeviceCreateWizardPage *finalPage = new HemeraDeviceCreateWizardPage(pathToHsdk, this);
    addPage(finalPage);
    finalPage->setCommitPage(true);

    QTimer::singleShot(0, this, SLOT(checkHsdk()));
}

HemeraDeviceConfigurationWizard::~HemeraDeviceConfigurationWizard()
{
}

void HemeraDeviceConfigurationWizard::checkHsdk()
{
    if (!checkHsdkInternal(this)) {
        reject();
    }
}

ProjectExplorer::IDevice::Ptr HemeraDeviceConfigurationWizard::device()
{
    QString deviceName = field(QStringLiteral("device.create.name")).toString();
    if (deviceName.isEmpty()) {
        qDebug() << "No such device name";
        return ProjectExplorer::IDevice::Ptr();
    }

    // Is it discovered?
    QItemSelectionModel *deviceSelector = field(QStringLiteral("device.configuration.selection")).value<QItemSelectionModel*>();
    QModelIndex selectedDevice = deviceSelector->currentIndex();
    Q_UNUSED(selectedDevice);

    return HemeraDevice::create(DeveloperMode::TargetManager::instance()->loadDevice(deviceName));
}


HemeraEmulatorConfigurationWizard::HemeraEmulatorConfigurationWizard(QWidget *parent, Qt::WindowFlags flags)
    : Utils::Wizard(parent, flags)
{
    setWindowTitle(tr("New Hemera Emulator Setup"));

    // Before we begin. We actually need to provide the path to hsdk, which has to be properly configured.
    HemeraSettingsPage *settingsPage = ExtensionSystem::PluginManager::getObject<HemeraSettingsPage>();
    if (!settingsPage) {
        return;
    }

    QString pathToHsdk = settingsPage->hsdkExecutable();

    addPage(new HemeraEmulatorConfigureWizardPage(this));
    HemeraEmulatorCreateWizardPage *finalPage = new HemeraEmulatorCreateWizardPage(pathToHsdk, this);
    addPage(finalPage);
    finalPage->setCommitPage(true);

    QTimer::singleShot(0, this, SLOT(checkHsdk()));
}

HemeraEmulatorConfigurationWizard::~HemeraEmulatorConfigurationWizard()
{
}

void HemeraEmulatorConfigurationWizard::checkHsdk()
{
    if (!HemeraDeviceConfigurationWizard::checkHsdkInternal(this)) {
        reject();
    }
}

ProjectExplorer::IDevice::Ptr HemeraEmulatorConfigurationWizard::emulator()
{
    QString emulatorName = field(QStringLiteral("emulator.create.name")).toString();
    if (emulatorName.isEmpty()) {
        return ProjectExplorer::IDevice::Ptr();
    }

    return HemeraEmulator::create(DeveloperMode::TargetManager::instance()->loadEmulator(emulatorName));
}

} // namespace Internal
} // namespace Hemera

