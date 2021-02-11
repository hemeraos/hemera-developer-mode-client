#ifndef HEMERACONSTANTS_H
#define HEMERACONSTANTS_H

namespace Hemera {
namespace Constants {

const char FLASH_IMAGE_ACTION_ID[] = "Hemera.Action.FlashImage";
const char START_STOP_EMULATOR_ACTION_ID[] = "Hemera.Action.StartStopEmulator";
const char MENU_ID[] = "Hemera.Menu";

const char HEMERA_DEVICE_TYPE[] = "Hemera.Device.Type";
const char HEMERA_DEVICE_TYPE_DEVICE[] = "Hemera.Device.Type.Device";
const char HEMERA_DEVICE_TYPE_EMULATOR[] = "Hemera.Device.Type.Emulator";
const char HEMERA_DEVICE_ID[] = "Hemera Device";
const char HEMERA_TARGET_NAME[] = "Hemera.Target.Name";
const char HEMERA_TARGET_ARCHITECTURE[] = "Hemera.Target.Architecture";

const char PROJECT_CONTEXT[]         = "Hemera.ProjectContext";
const char MIME_TYPE[]               = "application/x-hemeraproject";
const char RUN_CONFIGURE[]              = "Hemera.RunConfigure";
const char RUN_CONFIGURE_CONTEXT_MENU[] = "Hemera.RunConfigureContextMenu";

// Hemera Qt Support
const char HEMERA_QT[]   = "Hemera.QtVersion.Hemera";
const char FEATURE_HEMERA[] = "QtSupport.Wizards.Feature.Hemera";
const char FEATURE_EMULATOR[] = "QtSupport.Wizards.Feature.Emulator";
const char HEMERA_QT_PLATFORM[] = "Hemera";
const char HEMERA_QT_PLATFORM_TR[] = QT_TRANSLATE_NOOP("Hemera", "Hemera");

// Project
const char PROJECT_ID[] = "Hemera.Project";

// Build Configuration
const char BUILD_CONFIG_ID[] = "Hemera.BuildConfiguration";

// Deploy Configuration
const char DEPLOY_STEP_ID[] = "Hemera.DeployStep";
const char DEPLOY_CONFIG_ID[] = "Hemera.DeployConfiguration";
const char DEPLOY_CONFIG_PREFIX[] = "Hemera.DeployConfiguration.";

// Run Configuration
const char RUN_CONFIG_ID[]               = "Hemera.RunConfiguration";
const char RUN_CONFIG_PREFIX[]           = "Hemera.RunConfiguration:";
const char RUN_CONFIG_USER_WORKING_DIR[] = "Hemera.RunConfiguration.UserWorkingDirectory";
const char RUN_CONFIG_USE_TERMINAL_KEY[] = "Hemera.RunConfiguration.UseTerminal";
const char RUN_CONFIG_TITLE_KEY[]        = "Hemera.RunConfiguation.Title";
const char RUN_CONFIG_ARGUMENTS_KEY[]    = "Hemera.RunConfiguration.Arguments";
const char RUN_CONFIG_APPLICATION_ID[]   = "Hemera.RunConfiguration.ApplicationId";
const char RUN_CONFIG_STAR[]             = "Hemera.RunConfiguration.Star";

// ToolChain
const char TOOLCHAIN_ID[]   = "Hemera.ToolChain";
const char TOOLCHAIN_TYPE[] = "hemera";

// HsdkStep
const char HSDK_STEP_ID[]              = "Hemera.HsdkStep";
const char HSDK_STEP_BUILD_TARGETS[]   = "Hemera.HsdkStep.BuildTargets";
const char HSDK_STEP_CLEAN[]           = "Hemera.HsdkStep.Clean";
const char HSDK_STEP_ADDITIONAL_ARGS[] = "Hemera.HsdkStep.AdditionalArguments";

// Wrappers
#ifdef Q_OS_WIN
#define SCRIPT_EXTENSION ".cmd"
#else // Q_OS_WIN
#define SCRIPT_EXTENSION ""
#endif // Q_OS_WIN

const char HEMERA_WRAPPER_RPMBUILD[] = "rpmbuild" SCRIPT_EXTENSION;
const char HEMERA_WRAPPER_QMAKE[] = "qmake" SCRIPT_EXTENSION;
const char HEMERA_WRAPPER_MAKE[] = "make" SCRIPT_EXTENSION;
const char HEMERA_WRAPPER_GCC[] = "gcc" SCRIPT_EXTENSION;
const char HEMERA_WRAPPER_GDB[] = "gdb" SCRIPT_EXTENSION;
const char HEMERA_WRAPPER_DEPLOY[] = "deploy" SCRIPT_EXTENSION;
const char HEMERA_WRAPPER_RPM[] = "rpm" SCRIPT_EXTENSION;

const char HEMERA_SDK_FILENAME[] = "/qtcreator/hemerasdk.xml";
const char HEMERA_TARGETS_FILENAME[] = "/targets.xml";
const char HEMERA_DEVICES_FILENAME[] = "/devices.xml";
const char HEMERA_SDK_TOOLS[] = "/hemera-sdk-tools/";

// Others
const char HEMERA_EDITOR_ID[]           = "Hemera.Editor";
const char HEMERA_EDITOR_DISPLAY_NAME[] = "Hemera Editor";
const char C_HEMERAEDITOR[]             = "Hemera.Context.Editor";
const char M_CONTEXT[]                  = "Hemera.Editor.ContextMenu";
const char HEMERA_SUPPORT_FEATURE[]     = "Hemera.SupportFeature";
const char HEMERA_KIT_INFORMATION[]     = "Hemera.Kit.Information";
const char HEMERA_SDK_INSTALLDIR[] = "HemeraSDK.InstallDir";

} // namespace Hemera
} // namespace Constants

#endif // HEMERACONSTANTS_H

