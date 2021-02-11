#ifndef HEMERAPROJECTNODE_H
#define HEMERAPROJECTNODE_H

#include <projectexplorer/projectnodes.h>

namespace Hemera {
namespace DeveloperMode {
class HaManager;
}
namespace Internal {

class HemeraProjectNode : public ProjectExplorer::ProjectNode
{
    Q_OBJECT
    friend class HemeraProject;
public:
    HemeraProjectNode(const QString &filename, DeveloperMode::HaManager *manager);
    virtual bool hasBuildTargets() const;
    virtual QList<ProjectExplorer::ProjectAction> supportedActions(Node *node) const override;

    virtual bool canAddSubProject(const QString &proFilePath) const;

    virtual bool addSubProjects(const QStringList &proFilePaths);
    virtual bool removeSubProjects(const QStringList &proFilePaths);
    virtual bool addFiles( const QStringList &filePaths,
                          QStringList *notAdded = 0);
    virtual bool removeFiles(const QStringList &filePaths,
                             QStringList *notRemoved = 0);
    virtual bool deleteFiles(const QStringList &filePaths);
    virtual bool renameFile(const QString &filePath,
                            const QString &newFilePath);
    virtual QList<ProjectExplorer::RunConfiguration *> runConfigurationsFor(Node *node);

private:
    DeveloperMode::HaManager *m_haManager;
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERAPROJECTNODE_H
