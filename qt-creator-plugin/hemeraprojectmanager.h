#ifndef HEMERAPROJECTMANAGER_H
#define HEMERAPROJECTMANAGER_H

#include <projectexplorer/iprojectmanager.h>
#include <coreplugin/dialogs/ioptionspage.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectnodes.h>
#include <coreplugin/icontext.h>
#include <texteditor/codeassist/keywordscompletionassist.h>

#include <utils/environment.h>
#include <utils/pathchooser.h>

#include <QFuture>
#include <QStringList>
#include <QCheckBox>
#include <QDir>
#include <QVector>
#include <QAction>

#include "hsdkvalidator.h"

QT_FORWARD_DECLARE_CLASS(QLabel)

namespace Utils {
class QtcProcess;
}

namespace Hemera {
namespace Internal {

class HemeraSettingsPage;

class HemeraProjectManager : public ProjectExplorer::IProjectManager
{
    Q_OBJECT
public:
    HemeraProjectManager(HemeraSettingsPage *hemeraSettingsPage);

    virtual ProjectExplorer::Project *openProject(const QString &fileName, QString *errorString) override;
    virtual QString mimeType() const override;

    QString hsdkExecutable() const;
    bool isHsdkExecutableValid() const;

    void setHsdkExecutable(const QString &executable);

    static QString findDumperLibrary(const Utils::Environment &env);
private slots:
    void updateContextMenu(ProjectExplorer::Project *project, ProjectExplorer::Node *node);
    void runConfigure();
    void runHemeraContextMenu();
private:
    void runConfigure(ProjectExplorer::Project *project);
    HemeraSettingsPage *m_settingsPage;
    QAction *m_runConfigureAction;
    QAction *m_runConfigureActionContextMenu;
    ProjectExplorer::Project *m_contextProject;
};

class HemeraSettingsPage : public Core::IOptionsPage
{
    Q_OBJECT

public:
    HemeraSettingsPage();
    virtual ~HemeraSettingsPage();

    virtual QWidget *widget() override;
    virtual void apply() override;
    virtual void finish() override;

    QString hsdkExecutable() const;
    void setHsdkExecutable(const QString &executable);
    bool isHsdkExecutableValid() const;
    QStringList supportedHemeraSDKs() const;
    QString hsdkVersion() const;

    TextEditor::Keywords keywords();

private Q_SLOTS:
    void updateLabelValues();

private:
    void saveSettings() const;
    QString findHsdkExecutable() const;

    Utils::PathChooser *m_pathchooser;
    QPointer<QWidget> m_widget;
    QPointer<QLabel> m_sdksLabel;
    QPointer<QLabel> m_versionLabel;
    HsdkValidator m_hemeraValidatorForUser;
    HsdkValidator m_hemeraValidatorForSystem;
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERAPROJECTMANAGER_H
