#include "hemeraprojectnodes.h"

#include <hemeradevelopermodehamanager.h>

#include <coreplugin/mimedatabase.h>
#include <projectexplorer/nodesvisitor.h>
#include <projectexplorer/projectexplorerconstants.h>

namespace Hemera {
namespace Internal {

DeveloperMode::HaManager::FileCategory categoryForMimeType(const QString &mimeType)
{
    if (mimeType == QLatin1String(ProjectExplorer::Constants::CPP_HEADER_MIMETYPE)
            || mimeType == QLatin1String(ProjectExplorer::Constants::C_HEADER_MIMETYPE)) {
        // Nowhere!
        return DeveloperMode::HaManager::FileCategory::Unknown;
    }

    if (mimeType == QLatin1String(ProjectExplorer::Constants::CPP_SOURCE_MIMETYPE)
               || mimeType == QLatin1String(ProjectExplorer::Constants::C_SOURCE_MIMETYPE)) {
        return DeveloperMode::HaManager::FileCategory::SourceFiles;
    }

    if (mimeType == QLatin1String(ProjectExplorer::Constants::RESOURCE_MIMETYPE))
        return DeveloperMode::HaManager::FileCategory::QrcFiles;

    if (mimeType == QLatin1String(ProjectExplorer::Constants::FORM_MIMETYPE))
        return DeveloperMode::HaManager::FileCategory::FormFiles;

    if (mimeType == QLatin1String(ProjectExplorer::Constants::QML_MIMETYPE))
        return DeveloperMode::HaManager::FileCategory::ResourceFiles;

    // Resource
    return DeveloperMode::HaManager::FileCategory::ResourceFiles;
}

QStringList listForMimeType(const QString &mimeType, DeveloperMode::HaManager *manager)
{
    if (mimeType == QLatin1String(ProjectExplorer::Constants::CPP_HEADER_MIMETYPE)
            || mimeType == QLatin1String(ProjectExplorer::Constants::C_HEADER_MIMETYPE)) {
        // Nowhere!
        return QStringList();
    }

    if (mimeType == QLatin1String(ProjectExplorer::Constants::CPP_SOURCE_MIMETYPE)
               || mimeType == QLatin1String(ProjectExplorer::Constants::C_SOURCE_MIMETYPE)) {
        return manager->fileList(DeveloperMode::HaManager::FileCategory::SourceFiles);
    }

    if (mimeType == QLatin1String(ProjectExplorer::Constants::RESOURCE_MIMETYPE))
        return manager->fileList(DeveloperMode::HaManager::FileCategory::QrcFiles);

    if (mimeType == QLatin1String(ProjectExplorer::Constants::FORM_MIMETYPE))
        return manager->fileList(DeveloperMode::HaManager::FileCategory::FormFiles);

    if (mimeType == QLatin1String(ProjectExplorer::Constants::QML_MIMETYPE))
        return manager->fileList(DeveloperMode::HaManager::FileCategory::ResourceFiles);

    // Resource
    return manager->fileList(DeveloperMode::HaManager::FileCategory::ResourceFiles);
}

HemeraProjectNode::HemeraProjectNode(const QString &filename, DeveloperMode::HaManager *manager)
    : ProjectExplorer::ProjectNode(filename)
    , m_haManager(manager)
{
}

bool HemeraProjectNode::hasBuildTargets() const
{
    // TODO
    return true;
}

QList<ProjectExplorer::ProjectAction> HemeraProjectNode::supportedActions(Node *node) const
{
    Q_UNUSED(node);
    QList<ProjectExplorer::ProjectAction> actions;
    actions << ProjectExplorer::ProjectAction::AddNewFile;
    actions << ProjectExplorer::ProjectAction::AddExistingFile;
    actions << ProjectExplorer::ProjectAction::AddExistingDirectory;
    actions << ProjectExplorer::ProjectAction::EraseFile;
    actions << ProjectExplorer::ProjectAction::Rename;
    actions << ProjectExplorer::ProjectAction::RemoveFile;

    return actions;
}

// No subproject support
bool HemeraProjectNode::canAddSubProject(const QString &proFilePath) const
{
    Q_UNUSED(proFilePath)
    return false;
}

bool HemeraProjectNode::addSubProjects(const QStringList &proFilePaths)
{
    Q_UNUSED(proFilePaths)
    return false;
}

bool HemeraProjectNode::removeSubProjects(const QStringList &proFilePaths)
{
    Q_UNUSED(proFilePaths)
    return false;
}

bool HemeraProjectNode::addFiles(const QStringList &filePaths, QStringList *notAdded)
{
    ProjectExplorer::FindAllFilesVisitor visitor;
    accept(&visitor);
    const QStringList &allFiles = visitor.filePaths();

    typedef QMap<QString, QStringList> TypeFileMap;
    // Split into lists by file type and bulk-add them.
    TypeFileMap typeFileMap;
    foreach (const QString &file, filePaths) {
        const Core::MimeType mt = Core::MimeDatabase::findByFile(file);
        typeFileMap[mt.type()] << file;
    }

    bool success = true;
    foreach (const QString &type, typeFileMap.keys()) {
        if (categoryForMimeType(type) == DeveloperMode::HaManager::FileCategory::Unknown) {
            continue;
        }

        const QStringList typeFiles = typeFileMap.value(type);

        // Watch out watch out!
        if (categoryForMimeType(type) == DeveloperMode::HaManager::FileCategory::SourceFiles && !m_haManager->isBuildsystem()) {
            // No, thanks...
            if (notAdded) {
                notAdded->append(typeFiles);
            }
            success = false;
            continue;
        }

        QStringList uniqueFilePaths;
        foreach (const QString &file, typeFiles) {
            if (!allFiles.contains(file))
                uniqueFilePaths.append(file);
        }

        if (!m_haManager->addFiles(uniqueFilePaths, categoryForMimeType(type))) {
            if (notAdded) {
                notAdded->append(uniqueFilePaths);
            }
            success = false;
        }
    }

    // We need to force-update the project, just in case.
    emitNodeUpdated();

    return success;
}

bool HemeraProjectNode::removeFiles(const QStringList &filePaths,  QStringList *notRemoved)
{
    bool success = true;
    typedef QMap<QString, QStringList> TypeFileMap;
    // Split into lists by file type and bulk-add them.
    TypeFileMap typeFileMap;
    foreach (const QString &file, filePaths) {
        const Core::MimeType mt = Core::MimeDatabase::findByFile(file);
        typeFileMap[mt.type()] << file;
    }
    foreach (const QString &type, typeFileMap.keys()) {
        if (categoryForMimeType(type) == DeveloperMode::HaManager::FileCategory::Unknown) {
            continue;
        }
        const QStringList typeFiles = typeFileMap.value(type);
        if (!m_haManager->removeFiles(typeFiles, categoryForMimeType(type))) {
            if (notRemoved) {
                *notRemoved << typeFiles;
            }
            success = false;
        }
    }

    // We need to force-update the project, just in case.
    emitNodeUpdated();

    return success;
}

bool HemeraProjectNode::deleteFiles(const QStringList &filePaths)
{
    return removeFiles(filePaths);
}

bool HemeraProjectNode::renameFile(const QString &filePath, const QString &newFilePath)
{
    if (newFilePath.isEmpty())
        return false;

    QString mimetype = Core::MimeDatabase::findByFile(filePath).type();
    if (categoryForMimeType(mimetype) == DeveloperMode::HaManager::FileCategory::Unknown) {
        // Nothing to do, but we need to force update the project.
        emitNodeUpdated();
        return true;
    }

    return m_haManager->renameFile(filePath, newFilePath, categoryForMimeType(mimetype));
}

QList<ProjectExplorer::RunConfiguration *> HemeraProjectNode::runConfigurationsFor(Node *node)
{
    Q_UNUSED(node)
    return QList<ProjectExplorer::RunConfiguration *>();
}

} // namespace Internal
} // namespace Hemera
