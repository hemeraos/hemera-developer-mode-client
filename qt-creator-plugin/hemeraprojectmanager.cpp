#include "hemeraprojectmanager.h"

#include "hemeraopenprojectwizard.h"
#include "hemeraconstants.h"
#include "hemeraproject.h"

#include <utils/synchronousprocess.h>
#include <utils/qtcprocess.h>

#include <coreplugin/icore.h>
#include <coreplugin/id.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/target.h>
#include <utils/QtConcurrentTools>
#include <QtConcurrentRun>
#include <QCoreApplication>
#include <QSettings>
#include <QDateTime>
#include <QFormLayout>
#include <QBoxLayout>
#include <QDesktopServices>
#include <QApplication>
#include <QLabel>
#include <QGroupBox>
#include <QSpacerItem>

namespace Hemera {
namespace Internal {

HemeraProjectManager::HemeraProjectManager(HemeraSettingsPage *hemeraSettingsPage)
    : m_settingsPage(hemeraSettingsPage)
{
    ProjectExplorer::ProjectExplorerPlugin *projectExplorer = ProjectExplorer::ProjectExplorerPlugin::instance();
    connect(projectExplorer, SIGNAL(aboutToShowContextMenu(ProjectExplorer::Project*,ProjectExplorer::Node*)),
            this, SLOT(updateContextMenu(ProjectExplorer::Project*,ProjectExplorer::Node*)));

    Core::ActionContainer *mbuild =
            Core::ActionManager::actionContainer(ProjectExplorer::Constants::M_BUILDPROJECT);
    Core::ActionContainer *mproject =
            Core::ActionManager::actionContainer(ProjectExplorer::Constants::M_PROJECTCONTEXT);
    Core::ActionContainer *msubproject =
            Core::ActionManager::actionContainer(ProjectExplorer::Constants::M_SUBPROJECTCONTEXT);

    const Core::Context projectContext(Hemera::Constants::PROJECT_CONTEXT);

    m_runConfigureAction = new QAction(QIcon(), tr("Run Configure"), this);
    Core::Command *command = Core::ActionManager::registerAction(m_runConfigureAction,
                                                                 Constants::RUN_CONFIGURE, projectContext);
    command->setAttribute(Core::Command::CA_Hide);
    mbuild->addAction(command, ProjectExplorer::Constants::G_BUILD_DEPLOY);
    connect(m_runConfigureAction, SIGNAL(triggered()), this, SLOT(runConfigure()));

    m_runConfigureActionContextMenu = new QAction(QIcon(), tr("Run Hemera"), this);
    command = Core::ActionManager::registerAction(m_runConfigureActionContextMenu,
                                                  Constants::RUN_CONFIGURE_CONTEXT_MENU, projectContext);
    command->setAttribute(Core::Command::CA_Hide);
    mproject->addAction(command, ProjectExplorer::Constants::G_PROJECT_BUILD);
    msubproject->addAction(command, ProjectExplorer::Constants::G_PROJECT_BUILD);
    connect(m_runConfigureActionContextMenu, SIGNAL(triggered()), this, SLOT(runHemeraContextMenu()));

}

void HemeraProjectManager::updateContextMenu(ProjectExplorer::Project *project, ProjectExplorer::Node *node)
{
    Q_UNUSED(node);
    m_contextProject = project;
}

void HemeraProjectManager::runConfigure()
{
    runConfigure(ProjectExplorer::ProjectExplorerPlugin::currentProject());
}

void HemeraProjectManager::runHemeraContextMenu()
{
    runConfigure(m_contextProject);
}

void HemeraProjectManager::runConfigure(ProjectExplorer::Project *project)
{
    if (!project) {
        return;
    }

    HemeraProject *hemeraProject = qobject_cast<HemeraProject *>(project);
    if (!hemeraProject || !hemeraProject->activeTarget() || !hemeraProject->activeTarget()->activeBuildConfiguration()) {
        return;
    }

    if (!ProjectExplorer::ProjectExplorerPlugin::instance()->saveModifiedFiles()) {
        return;
    }

    HemeraBuildConfiguration *bc
            = static_cast<HemeraBuildConfiguration *>(hemeraProject->activeTarget()->activeBuildConfiguration());

    HemeraBuildInfo info(bc);

    HemeraOpenProjectWizard copw(this, HemeraOpenProjectWizard::WantToUpdate, &info);
    if (copw.exec() == QDialog::Accepted) {
        hemeraProject->parseHaFile();
    }
}

ProjectExplorer::Project *HemeraProjectManager::openProject(const QString &fileName, QString *errorString)
{
    qDebug() << "HemeraProjectManager::openProject" << fileName;

    if (!QFileInfo(fileName).isFile()) {
        if (errorString) {
            *errorString = tr("Failed opening project '%1': Project is not a file").arg(fileName);
        }
        return 0;
    }

    return new HemeraProject(this, fileName);
}

QString HemeraProjectManager::mimeType() const
{
    return QLatin1String(Constants::MIME_TYPE);
}

QString HemeraProjectManager::hsdkExecutable() const
{
    return m_settingsPage->hsdkExecutable();
}

bool HemeraProjectManager::isHsdkExecutableValid() const
{
    return m_settingsPage->isHsdkExecutableValid();
}

void HemeraProjectManager::setHsdkExecutable(const QString &executable)
{
    m_settingsPage->setHsdkExecutable(executable);
}

/////
// HemeraSettingsPage
////

HemeraSettingsPage::HemeraSettingsPage()
    :  m_pathchooser(0)
{
    setId("Z.Hemera");
    setDisplayName(tr("Hemera"));
    setCategory(ProjectExplorer::Constants::PROJECTEXPLORER_SETTINGS_CATEGORY);
    setDisplayCategory(QCoreApplication::translate("ProjectExplorer",
       ProjectExplorer::Constants::PROJECTEXPLORER_SETTINGS_TR_CATEGORY));
    setCategoryIcon(QLatin1String(ProjectExplorer::Constants::PROJECTEXPLORER_SETTINGS_CATEGORY_ICON));

    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(QLatin1String("HemeraSettings"));
    m_hemeraValidatorForUser.setHsdkExecutable(settings->value(QLatin1String("hsdkExecutable")).toString());
    settings->endGroup();

    m_hemeraValidatorForSystem.setHsdkExecutable(findHsdkExecutable());
}

bool HemeraSettingsPage::isHsdkExecutableValid() const
{
    if (m_hemeraValidatorForUser.isValid())
        return true;

    return m_hemeraValidatorForSystem.isValid();
}

HemeraSettingsPage::~HemeraSettingsPage()
{
    m_hemeraValidatorForUser.cancel();
    m_hemeraValidatorForSystem.cancel();
}

QString HemeraSettingsPage::findHsdkExecutable() const
{
    QString hsdk;
    if (Utils::HostOsInfo::isWindowsHost()) {
        hsdk = QStringLiteral("hsdk.exe");
    } else {
        hsdk = QStringLiteral("hsdk");
    }

    QString tryAndFind = Utils::Environment::systemEnvironment().searchInPath(hsdk).toString();
    if (!tryAndFind.isEmpty()) {
        return tryAndFind;
    }

    if (QFile::exists(QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + QDir::separator() + hsdk))) {
        return QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + QDir::separator() + hsdk);
    }

    return QString();
}

void HemeraSettingsPage::updateLabelValues()
{
    if (m_sdksLabel) {
        m_sdksLabel->setText(supportedHemeraSDKs().join(QStringLiteral(", ")));
    }

    if (m_versionLabel) {
        m_versionLabel->setText(hsdkVersion());
    }
}

QWidget *HemeraSettingsPage::widget()
{
    if (!m_widget) {
        m_widget = new QWidget;
        m_sdksLabel = new QLabel;
        m_versionLabel = new QLabel;
        QVBoxLayout *vertLayout = new QVBoxLayout(m_widget);
        QGroupBox *hsdkGroup = new QGroupBox(tr("hsdk settings"));
        vertLayout->addWidget(hsdkGroup);
        QFormLayout *formLayout = new QFormLayout(hsdkGroup);
        formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
        m_pathchooser = new Utils::PathChooser;
        m_pathchooser->setExpectedKind(Utils::PathChooser::ExistingCommand);
        m_pathchooser->setHistoryCompleter(QLatin1String("Hemera.Command.History"));
        formLayout->addRow(tr("Executable:"), m_pathchooser);
        formLayout->addRow(tr("Supported SDKs:"), m_sdksLabel);
        formLayout->addRow(tr("Version:"), m_versionLabel);
        formLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding));

        vertLayout->addStretch();
    }
    m_pathchooser->setPath(m_hemeraValidatorForUser.hsdkExecutable());
    updateLabelValues();
    return m_widget;
}

void HemeraSettingsPage::saveSettings() const
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(QLatin1String("HemeraSettings"));
    settings->setValue(QLatin1String("hsdkExecutable"), m_hemeraValidatorForUser.hsdkExecutable());
    settings->endGroup();
}

void HemeraSettingsPage::apply()
{
    if (!m_pathchooser) { // page was never shown
        return;
    }

    if (m_hemeraValidatorForUser.hsdkExecutable() != m_pathchooser->path()) {
        m_hemeraValidatorForUser.setHsdkExecutable(m_pathchooser->path());
    }

    saveSettings();
    updateLabelValues();
}

void HemeraSettingsPage::finish()
{
    if (!m_widget.isNull()) {
        m_widget->deleteLater();
    }
}

QString HemeraSettingsPage::hsdkExecutable() const
{
    if (!isHsdkExecutableValid()) {
        return QString();
    }

    if (m_hemeraValidatorForUser.isValid()) {
        return m_hemeraValidatorForUser.hsdkExecutable();
    }

    if (m_hemeraValidatorForSystem.isValid()) {
        return m_hemeraValidatorForSystem.hsdkExecutable();
    }

    return QString();
}

void HemeraSettingsPage::setHsdkExecutable(const QString &executable)
{
    if (m_hemeraValidatorForUser.hsdkExecutable() == executable)
        return;
    m_hemeraValidatorForUser.setHsdkExecutable(executable);
    updateLabelValues();
}

QStringList HemeraSettingsPage::supportedHemeraSDKs() const
{
    if (m_hemeraValidatorForUser.isValid()) {
        return m_hemeraValidatorForUser.supportedHemeraSDKs();
    }

    if (m_hemeraValidatorForSystem.isValid()) {
        return m_hemeraValidatorForSystem.supportedHemeraSDKs();
    }

    return QStringList();
}

QString HemeraSettingsPage::hsdkVersion() const
{
    if (m_hemeraValidatorForUser.isValid()) {
        return m_hemeraValidatorForUser.hsdkVersion();
    }

    if (m_hemeraValidatorForSystem.isValid()) {
        return m_hemeraValidatorForSystem.hsdkVersion();
    }

    return QString();
}

TextEditor::Keywords HemeraSettingsPage::keywords()
{
//    if (m_hemeraValidatorForUser.isValid())
//        return m_hemeraValidatorForUser.keywords();

//    if (m_hemeraValidatorForSystem.isValid())
//        return m_hemeraValidatorForSystem.keywords();

    return TextEditor::Keywords(QStringList(), QStringList(), QMap<QString, QStringList>());
}

} // namespace Internal
} // namespace Hemera
