#include "hemeraplugin.h"
#include "hemeraconstants.h"

// Autoreleased objects
#include "hemerabuildconfiguration.h"
#include "hemeradeployconfiguration.h"
#include "hemeradeploystep.h"
#include "hemeraprojectmanager.h"
#include "hemerarunconfiguration.h"
#include "hemeratargetfactory.h"
#include "hemeraqtversion.h"
#include "hemerasdkmanager.h"
#include "hemeratoolchain.h"
#include "hsdkstep.h"

// Other objects
#include "hemeradevice.h"
#include "hemeraemulator.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>

#include <projectexplorer/devicesupport/devicemanager.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

#include <QtPlugin>

namespace Hemera {
namespace Internal {

HemeraPlugin::HemeraPlugin()
{
    setObjectName(QLatin1String("HemeraPlugin"));
}

HemeraPlugin::~HemeraPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
    removeObject(this);
}

bool HemeraPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.

    Q_UNUSED(arguments)

    // Add mimetypes
    if (!Core::MimeDatabase::addMimeTypes(QLatin1String(":hemera/Hemera.mimetypes.xml"), errorMessage)) {
       return false;
    }

    // Register ourselves.
    addObject(this);

    // Settings page - autoreleased, and needed elsewhere.
    HemeraSettingsPage *hsp = new HemeraSettingsPage();
    addAutoReleasedObject(hsp);

    // Our main entry point.
    addAutoReleasedObject(new HemeraSDKManager);

    // Factories and Managers, for Creator.
    addAutoReleasedObject(new HemeraBuildConfigurationFactory);
    addAutoReleasedObject(new HemeraDeployConfigurationFactory);
    addAutoReleasedObject(new HemeraDeployStepFactory);
    addAutoReleasedObject(new HemeraProjectManager(hsp));
    addAutoReleasedObject(new HemeraQtVersionFactory);
    addAutoReleasedObject(new HemeraRunConfigurationFactory);
    addAutoReleasedObject(new HemeraRunControlFactory);
    addAutoReleasedObject(new HemeraTargetFactory);
    addAutoReleasedObject(new HemeraToolChainFactory);
    addAutoReleasedObject(new HsdkStepFactory);

    QAction *action = new QAction(tr("Flash Hemera image to Device"), this);
    Core::Command *cmd = Core::ActionManager::registerAction(action, Constants::FLASH_IMAGE_ACTION_ID,
                                                             Core::Context(Core::Constants::C_GLOBAL));
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Meta+F")));

    Core::ActionContainer *menu = Core::ActionManager::createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("Hemera"));
    menu->addAction(cmd);

    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    return true;
}

void HemeraPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag HemeraPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    return SynchronousShutdown;
}

}
}
