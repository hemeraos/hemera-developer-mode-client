#ifndef HEMERAPROJECT_H
#define HEMERAPROJECT_H

#include "hemeraprojectmanager.h"
#include "hemeraprojectnodes.h"
#include "hemerabuildconfiguration.h"

#include <projectexplorer/project.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/namedwidget.h>
#include <coreplugin/idocument.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>

#include <QXmlStreamReader>
#include <QPushButton>
#include <QLineEdit>

QT_BEGIN_NAMESPACE
class QFileSystemWatcher;
QT_END_NAMESPACE

namespace ProjectExplorer { class Target; }

namespace Hemera {
namespace DeveloperMode {
class HaManager;
}
namespace Internal {

class HaFile;
class HemeraBuildSettingsWidget;

struct HemeraBuildTarget
{
    QString title;
    QString packageName; // Should be unique for every kit.
    bool    library;
    QString workingDirectory;
    QString sourceDirectory;

    // code model
    QStringList includeFiles;
    QStringList compilerOptions;
    QByteArray defines;
    QStringList files;

    void clear();
};

class HemeraProject : public ProjectExplorer::Project
{
    Q_OBJECT
    // for changeBuildDirectory
    friend class HemeraBuildSettingsWidget;
public:
    HemeraProject(HemeraProjectManager *manager, const QString &filename);
    ~HemeraProject();

    QString displayName() const;
    Core::Id id() const;
    Core::IDocument *document() const;
    HemeraProjectManager *projectManager() const;

    ProjectExplorer::ProjectNode *rootProjectNode() const;

    bool supportsKit(ProjectExplorer::Kit *k, QString *errorMessage = 0) const;
    virtual bool needsSpecialDeployment() const override { return true; }

    QStringList files(FilesMode fileMode) const;
    QStringList buildTargetTitles(bool runnable = true) const;
    QList<HemeraBuildTarget> buildTargets() const;
    bool hasBuildTarget(const QString &title) const;

    HemeraBuildTarget buildTargetForTitle(const QString &title);

    bool isProjectFile(const QString &fileName);

    bool parseHaFile();

signals:
    /// emitted after parsing
    void buildTargetsChanged();

protected:
    bool fromMap(const QVariantMap &map);
    bool setupTarget(ProjectExplorer::Target *t);

    // called by HemeraBuildSettingsWidget
    void changeBuildDirectory(HemeraBuildConfiguration *bc, const QString &newBuildDirectory);

private slots:
    void fileChanged(const QString &fileName);
    void activeTargetWasChanged(ProjectExplorer::Target *target);
    void changeActiveBuildConfiguration(ProjectExplorer::BuildConfiguration*);

    void updateDeploymentDataForTarget(ProjectExplorer::Target *target);
    void updateRunConfigurationForTarget(ProjectExplorer::Target *t);
    void updateRunConfigurations();

private:
    void buildTree(HemeraProjectNode *rootNode, QList<ProjectExplorer::FileNode *> list);
    void gatherFileNodes(ProjectExplorer::FolderNode *parent, QList<ProjectExplorer::FileNode *> &list);
    ProjectExplorer::FolderNode *findOrCreateFolder(HemeraProjectNode *rootNode, QString directory);
    void createUiCodeModelSupport();
    void refreshCppCodeModel();
    QString uiHeaderFile(const QString &uiFile);

    QStringList directoryEntryList(const QDir& dir, const QString &fileName = QString());

    HemeraProjectManager *m_manager;
    ProjectExplorer::Target *m_activeTarget;
    QString m_fileName;
    DeveloperMode::HaManager *m_haManager;
    HaFile *m_file;
    QString m_projectName;

    // TODO probably need a Hemera specific node structure
    HemeraProjectNode *m_rootNode;
    QStringList m_files;
    QList<HemeraBuildTarget> m_buildTargets;
    QFileSystemWatcher *m_watcher;
    QSet<QString> m_watchedFiles;
    QFuture<void> m_codeModelFuture;
};

class HaFile : public Core::IDocument
{
    Q_OBJECT

public:
    HaFile(HemeraProject *parent, DeveloperMode::HaManager *haManager);

    bool save(QString *errorString, const QString &fileName, bool autoSave) override;

    QString defaultPath() const override;
    QString suggestedFileName() const override;

    bool isModified() const override;
    bool isSaveAsAllowed() const override;

    ReloadBehavior reloadBehavior(ChangeTrigger state, ChangeType type) const override;
    bool reload(QString *errorString, ReloadFlag flag, ChangeType type) override;

private:
    HemeraProject *m_project;
    DeveloperMode::HaManager *m_haManager;
};

class HemeraBuildSettingsWidget : public ProjectExplorer::NamedWidget
{
    Q_OBJECT
public:
    HemeraBuildSettingsWidget(HemeraBuildConfiguration *bc);

private slots:
    void openChangeBuildDirectoryDialog();
    void runHemera();
private:
    QLineEdit *m_pathLineEdit;
    QPushButton *m_changeButton;
    HemeraBuildConfiguration *m_buildConfiguration;
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERAPROJECT_H
