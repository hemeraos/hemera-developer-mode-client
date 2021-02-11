#include "hemeraproject.h"

#include "hemerabuildconfiguration.h"
#include "hemeraconstants.h"
#include "hemerakitinformation.h"
#include "hemeraprojectnodes.h"
#include "hemerarunconfiguration.h"
#include "hemeraopenprojectwizard.h"
#include "hemeraconstants.h"
#include "hemeratarget.h"

#include <hemeradevelopermodehamanager.h>

#include <coreplugin/icore.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/headerpath.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/buildmanager.h>
#include <projectexplorer/buildtargetinfo.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/target.h>
#include <projectexplorer/deployconfiguration.h>
#include <projectexplorer/deploymentdata.h>
#include <projectexplorer/projectmacroexpander.h>
#include <projectexplorer/toolchainmanager.h>
#include <qtsupport/customexecutablerunconfiguration.h>
#include <qtsupport/baseqtversion.h>
#include <qtsupport/qtkitinformation.h>
#include <qtsupport/uicodemodelsupport.h>
#include <cpptools/cppmodelmanager.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/qtcassert.h>
#include <utils/stringutils.h>
#include <utils/hostosinfo.h>
#include <coreplugin/icore.h>
#include <coreplugin/infobar.h>
#include <coreplugin/documentmanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/variablemanager.h>

#include <QDebug>
#include <QDir>
#include <QFormLayout>
#include <QFileSystemWatcher>

namespace Hemera {
namespace Internal {

// QtCreator Hemera Generator wishlist:
// Which make targets we need to build to get all executables
// What is the make we need to call
// What is the actual compiler executable
// DEFINES

// Open Questions
// Who sets up the environment for cl.exe ? INCLUDEPATH and so on

bool sortNodesByPath(ProjectExplorer::Node *a, ProjectExplorer::Node *b)
{
    return a->path() < b->path();
}

/*!
  \class HemeraProject
*/
HemeraProject::HemeraProject(HemeraProjectManager *manager, const QString &fileName)
    : ProjectExplorer::Project()
    , m_manager(manager)
    , m_activeTarget(0)
    , m_fileName(fileName)
    , m_haManager(new DeveloperMode::HaManager(fileName, this))
    , m_file(new HaFile(this, m_haManager))
    , m_rootNode(new HemeraProjectNode(fileName, m_haManager))
{
    setId(Constants::PROJECT_ID);
    setProjectContext(Core::Context(Hemera::Constants::PROJECT_CONTEXT));
    setRequiredKitMatcher(HemeraKitInformation::hemeraKitMatcher());

    m_projectName = QFileInfo(fileName).absoluteDir().dirName();

    connect(this, SIGNAL(buildTargetsChanged()),
            this, SLOT(updateRunConfigurations()));
    connect(this, SIGNAL(addedTarget(ProjectExplorer::Target*)),
            this, SLOT(updateDeploymentDataForTarget(ProjectExplorer::Target*)));
    connect(m_haManager, &DeveloperMode::HaManager::readyChanged, this, &HemeraProject::parseHaFile);

    // We need to watch the root node for changes
    ProjectExplorer::NodesWatcher *watcher = new ProjectExplorer::NodesWatcher(this);
    m_rootNode->registerWatcher(watcher);
    connect(watcher, &ProjectExplorer::NodesWatcher::nodeUpdated, this, &HemeraProject::parseHaFile, Qt::QueuedConnection);
}

HemeraProject::~HemeraProject()
{
    m_codeModelFuture.cancel();
    delete m_rootNode;
}

void HemeraProject::activeTargetWasChanged(ProjectExplorer::Target *target)
{
    if (m_activeTarget) {
        disconnect(m_activeTarget, SIGNAL(activeBuildConfigurationChanged(ProjectExplorer::BuildConfiguration*)),
                   this, SLOT(changeActiveBuildConfiguration(ProjectExplorer::BuildConfiguration*)));
    }

    m_activeTarget = target;

    if (!m_activeTarget)
        return;

    connect(m_activeTarget, SIGNAL(activeBuildConfigurationChanged(ProjectExplorer::BuildConfiguration*)),
            this, SLOT(changeActiveBuildConfiguration(ProjectExplorer::BuildConfiguration*)));

    changeActiveBuildConfiguration(m_activeTarget->activeBuildConfiguration());
}

HemeraBuildTarget HemeraProject::buildTargetForTitle(const QString &title)
{
    foreach (const HemeraBuildTarget &ct, m_buildTargets)
        if (ct.title == title)
            return ct;
    return HemeraBuildTarget();
}

QList<HemeraBuildTarget> HemeraProject::buildTargets() const
{
    return m_buildTargets;
}

void HemeraProject::buildTree(HemeraProjectNode *rootNode, QList<ProjectExplorer::FileNode *> newList)
{
    // Gather old list
    QList<ProjectExplorer::FileNode *> oldList;
    gatherFileNodes(rootNode, oldList);
    qSort(oldList.begin(), oldList.end(), sortNodesByPath);
    qSort(newList.begin(), newList.end(), sortNodesByPath);

    // generate added and deleted list
    QList<ProjectExplorer::FileNode *>::const_iterator oldIt  = oldList.constBegin();
    QList<ProjectExplorer::FileNode *>::const_iterator oldEnd = oldList.constEnd();
    QList<ProjectExplorer::FileNode *>::const_iterator newIt  = newList.constBegin();
    QList<ProjectExplorer::FileNode *>::const_iterator newEnd = newList.constEnd();

    QList<ProjectExplorer::FileNode *> added;
    QList<ProjectExplorer::FileNode *> deleted;

    while (oldIt != oldEnd && newIt != newEnd) {
        if ( (*oldIt)->path() == (*newIt)->path()) {
            delete *newIt;
            ++oldIt;
            ++newIt;
        } else if ((*oldIt)->path() < (*newIt)->path()) {
            deleted.append(*oldIt);
            ++oldIt;
        } else {
            added.append(*newIt);
            ++newIt;
        }
    }

    while (oldIt != oldEnd) {
        deleted.append(*oldIt);
        ++oldIt;
    }

    while (newIt != newEnd) {
        added.append(*newIt);
        ++newIt;
    }

    // add added nodes
    foreach (ProjectExplorer::FileNode *fn, added) {
//        qDebug()<<"added"<<fn->path();
        // Get relative path to rootNode
        QString parentDir = QFileInfo(fn->path()).absolutePath();
        ProjectExplorer::FolderNode *folder = findOrCreateFolder(rootNode, parentDir);
        folder->addFileNodes(QList<ProjectExplorer::FileNode *>()<< fn);
    }

    // remove old file nodes and check whether folder nodes can be removed
    foreach (ProjectExplorer::FileNode *fn, deleted) {
        ProjectExplorer::FolderNode *parent = fn->parentFolderNode();
//        qDebug()<<"removed"<<fn->path();
        parent->removeFileNodes(QList<ProjectExplorer::FileNode *>() << fn);
        // Check for empty parent
        while (parent->subFolderNodes().isEmpty() && parent->fileNodes().isEmpty()) {
            ProjectExplorer::FolderNode *grandparent = parent->parentFolderNode();
            grandparent->removeFolderNodes(QList<ProjectExplorer::FolderNode *>() << parent);
            parent = grandparent;
            if (parent == rootNode)
                break;
        }
    }
}

void HemeraProject::changeActiveBuildConfiguration(ProjectExplorer::BuildConfiguration *bc)
{
    if (!bc)
        return;

    HemeraBuildConfiguration *hemerabc = static_cast<HemeraBuildConfiguration *>(bc);

    // Pop up a dialog asking the user to rerun hsdk configure???
    /*QString cbpFile = HemeraProjectManager::findCbpFile(QDir(bc->buildDirectory().toString()));
    QFileInfo cbpFileFi(cbpFile);
    HemeraOpenProjectWizard::Mode mode = HemeraOpenProjectWizard::Nothing;
    if (!cbpFileFi.exists()) {
        mode = HemeraOpenProjectWizard::NeedToCreate;
    } else {
        foreach (const QString &file, m_watchedFiles) {
            if (QFileInfo(file).lastModified() > cbpFileFi.lastModified()) {
                mode = HemeraOpenProjectWizard::NeedToUpdate;
                break;
            }
        }
    }

    if (mode != HemeraOpenProjectWizard::Nothing) {
        HemeraBuildInfo info(hemerabc);
        HemeraOpenProjectWizard copw(m_manager, mode, &info);
    }*/

    // reparse
    parseHaFile();
}

void HemeraProject::changeBuildDirectory(HemeraBuildConfiguration *bc, const QString &newBuildDirectory)
{
    bc->setBuildDirectory(Utils::FileName::fromString(newBuildDirectory));
    parseHaFile();
}

void HemeraProject::createUiCodeModelSupport()
{
    QHash<QString, QString> uiFileHash;

    // Find all ui files
    foreach (const QString &uiFile, m_files) {
        if (uiFile.endsWith(QLatin1String(".ui"))) {
            uiFileHash.insert(uiFile, uiHeaderFile(uiFile));
        }
    }

    QtSupport::UiCodeModelManager::update(this, uiFileHash);
}

void HemeraProject::refreshCppCodeModel()
{
    CppTools::CppModelManager *modelManager =
            CppTools::CppModelManager::instance();

    if (!modelManager) {
        qDebug() << "Could not find model manager!!";
        return;
    }

    m_codeModelFuture.cancel();

    CppTools::ProjectInfo pInfo = modelManager->projectInfo(this);
    pInfo.clearProjectParts();

    CppTools::ProjectPartBuilder ppBuilder(pInfo);
    ppBuilder.setProjectFile(m_haManager->filePath());
    ppBuilder.setDisplayName(displayName());
    ppBuilder.setQtVersion(CppTools::ProjectPart::Qt5);
    ppBuilder.setCxxFlags(QStringList() << QLatin1String("-std=c++11"));

    // Headers now.
    CppTools::ProjectPart::HeaderPaths headers;
    {
        CppTools::ProjectPart::HeaderPath hemeraHeaders(QDir::toNativeSeparators(Core::ICore::resourcePath() + QStringLiteral("/hemera/includes")),
                                                        CppTools::ProjectPart::HeaderPath::IncludePath);
        headers << hemeraHeaders;

        // Each Hemera dir then is another candidate for header paths.
        QDir hemeraIncludesDir(QDir::toNativeSeparators(Core::ICore::resourcePath() + QStringLiteral("/hemera/includes")));

        for (const QFileInfo &entry : hemeraIncludesDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            headers << CppTools::ProjectPart::HeaderPath(entry.absolutePath(), CppTools::ProjectPart::HeaderPath::IncludePath);
        }
    }

    ppBuilder.setHeaderPaths(headers);

    const QList<Core::Id> languages = ppBuilder.createProjectPartsForFiles(files(AllFiles));
    for (const Core::Id &language : languages) {
        setProjectLanguage(language, true);
    }

    pInfo.finish();

    m_codeModelFuture = modelManager->updateProjectInfo(pInfo);
}

QStringList HemeraProject::buildTargetTitles(bool runnable) const
{
    QStringList results;
    foreach (const HemeraBuildTarget &ct, m_buildTargets) {
        if (runnable && (ct.packageName.isEmpty() || ct.library))
            continue;
        results << ct.title;
    }
    return results;
}

QStringList HemeraProject::directoryEntryList(const QDir& dir, const QString &fileName)
{
    if (!dir.exists()) {
        return QStringList();
    }

    QStringList files;

    QStringList fileNames = dir.entryList(QStringList() << (fileName.isEmpty() ? QLatin1String("*") : fileName), QDir::Files | QDir::NoSymLinks);
    Q_FOREACH(QString fileName, fileNames) {
        files += dir.absoluteFilePath(fileName);
    }

    QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name);
    Q_FOREACH(QString subdir, subdirs) {
        files += directoryEntryList(dir.absoluteFilePath(subdir), fileName);
    }

    files.sort();
    return files;
}

QString HemeraProject::displayName() const
{
    return m_projectName;
}

Core::IDocument *HemeraProject::document() const
{
    return m_file;
}

ProjectExplorer::FolderNode *HemeraProject::findOrCreateFolder(HemeraProjectNode *rootNode, QString directory)
{
    QString relativePath = QDir(QFileInfo(rootNode->path()).path()).relativeFilePath(directory);
    QStringList parts = relativePath.split(QLatin1Char('/'), QString::SkipEmptyParts);
    ProjectExplorer::FolderNode *parent = rootNode;
    QString path = QFileInfo(rootNode->path()).path();
    foreach (const QString &part, parts) {
        path += QLatin1Char('/');
        path += part;
        // Find folder in subFolders
        bool found = false;
        foreach (ProjectExplorer::FolderNode *folder, parent->subFolderNodes()) {
            if (folder->path() == path) {
                // yeah found something :)
                parent = folder;
                found = true;
                break;
            }
        }
        if (!found) {
            // No FolderNode yet, so create it
            ProjectExplorer::FolderNode *tmp = new ProjectExplorer::FolderNode(path);
            tmp->setDisplayName(part);
            parent->addFolderNodes(QList<ProjectExplorer::FolderNode *>() << tmp);
            parent = tmp;
        }
    }
    return parent;
}

void HemeraProject::fileChanged(const QString &fileName)
{
    Q_UNUSED(fileName)

    parseHaFile();
}

QStringList HemeraProject::files(FilesMode fileMode) const
{
    Q_UNUSED(fileMode)
    return m_files;
}

bool HemeraProject::fromMap(const QVariantMap &map)
{
    qDebug() << "HemeraProject::fromMap" << map.size();

    if (!Project::fromMap(map))
        return false;

    bool hasUserFile = activeTarget();

    if (!hasUserFile) {
        qDebug() << "HemeraProject::fromMap: NO ACTIVE TARGET";
//        HemeraOpenProjectWizard copw(m_manager, projectDirectory(), Utils::Environment::systemEnvironment());
//        if (copw.exec() != QDialog::Accepted)
//            return false;

        ProjectExplorer::Kit *kit = 0;
        QList<ProjectExplorer::Kit*> kitList = ProjectExplorer::KitManager::kits();

        Q_FOREACH (ProjectExplorer::Kit *k, kitList) {
            if (supportsKit(k)) {
                kit = k;
                break;
            }
        }

        if (!kit) {
            return false;
        }

        ProjectExplorer::Target *target = createTarget(kit);
        addTarget(target);
    }
//    else {
//        qDebug() << "HemeraProject::fromMap: ACTIVE TARGET FOUND";

//        // We have a user file, but we could still be missing the cbp file
//        // or simply run createXml with the saved settings
//        QFileInfo sourceFileInfo(m_fileName);
//        HemeraBuildConfiguration *activeBC = qobject_cast<HemeraBuildConfiguration *>(activeTarget()->activeBuildConfiguration());
//        if (!activeBC)
//            return false;
//        QString cbpFile = HemeraProjectManager::findCbpFile(QDir(activeBC->buildDirectory().toString()));
//        QFileInfo cbpFileFi(cbpFile);

//        HemeraOpenProjectWizard::Mode mode = HemeraOpenProjectWizard::Nothing;
//        if (!cbpFileFi.exists())
//            mode = HemeraOpenProjectWizard::NeedToCreate;
//        else if (cbpFileFi.lastModified() < sourceFileInfo.lastModified())
//            mode = HemeraOpenProjectWizard::NeedToUpdate;

//        if (mode != HemeraOpenProjectWizard::Nothing) {
//            HemeraBuildInfo info(activeBC);
//            HemeraOpenProjectWizard copw(m_manager, mode, &info);
//            if (copw.exec() != QDialog::Accepted)
//                return false;
//        }
//    }

    parseHaFile();

    m_activeTarget = activeTarget();
    if (m_activeTarget) {
        connect(m_activeTarget, SIGNAL(activeBuildConfigurationChanged(ProjectExplorer::BuildConfiguration*)),
                this,           SLOT(changeActiveBuildConfiguration(ProjectExplorer::BuildConfiguration*)));
    }

    connect(this, SIGNAL(activeTargetChanged(ProjectExplorer::Target*)),
            this, SLOT(activeTargetWasChanged(ProjectExplorer::Target*)));

    return true;
}

void HemeraProject::gatherFileNodes(ProjectExplorer::FolderNode *parent, QList<ProjectExplorer::FileNode *> &list)
{
    foreach (ProjectExplorer::FolderNode *folder, parent->subFolderNodes())
        gatherFileNodes(folder, list);
    foreach (ProjectExplorer::FileNode *file, parent->fileNodes())
        list.append(file);
}

bool HemeraProject::hasBuildTarget(const QString &title) const
{
    foreach (const HemeraBuildTarget &ct, m_buildTargets) {
        if (ct.title == title)
            return true;
    }
    return false;
}

Core::Id HemeraProject::id() const
{
    return Core::Id(Constants::PROJECT_ID);
}

bool HemeraProject::isProjectFile(const QString &fileName)
{
    return m_watchedFiles.contains(fileName);
}

bool HemeraProject::parseHaFile()
{
    if (!activeTarget() ||
        !activeTarget()->activeBuildConfiguration()) {
        return false;
    }

    if (!m_haManager->isReady()) {
        return false;
    }

    // Get on with names and stuff
    QString projectName = QStringLiteral("%1 (%2)").arg(m_haManager->applicationName(), m_haManager->applicationId());
    if (displayName() != projectName) {
        m_projectName = projectName;
        Q_EMIT displayNameChanged();
    }

    QDir prjDir(projectDirectory().toString());
    QStringList sourceFiles = m_haManager->fileList(DeveloperMode::HaManager::FileCategory::SourceFiles);
    // No other way than this. TODO: make this slightly better
    QStringList headerFiles = directoryEntryList(prjDir, QLatin1String("*.h"));
    QStringList qmlFiles;
    QStringList resourceFiles = m_haManager->fileList(DeveloperMode::HaManager::FileCategory::ResourceFiles);
    QStringList::iterator rIt = resourceFiles.begin();
    while (rIt != resourceFiles.end()) {
        if ((*rIt).endsWith(QStringLiteral(".qml"))) {
            qmlFiles << *rIt;
            rIt = resourceFiles.erase(rIt);
        } else {
            ++rIt;
        }
    }

    QStringList uiFiles     = m_haManager->fileList(DeveloperMode::HaManager::FileCategory::FormFiles);
    QStringList qrcFiles    = m_haManager->fileList(DeveloperMode::HaManager::FileCategory::QrcFiles);

    m_files.clear();
    m_files << sourceFiles << headerFiles << qmlFiles << resourceFiles << uiFiles << qrcFiles;

    QList<ProjectExplorer::FileNode*> fileList;
    fileList.append(new ProjectExplorer::FileNode(m_fileName, ProjectExplorer::ProjectFileType, false));
    for (const QString &file : sourceFiles) {
        // Prepend the project directory
        fileList.append(new ProjectExplorer::FileNode(prjDir.absolutePath() + QDir::separator() +file, ProjectExplorer::SourceType, false));
    }
    for (const QString &file : headerFiles) {
        fileList.append(new ProjectExplorer::FileNode(file, ProjectExplorer::HeaderType, false));
    }
    for (const QString &file : qmlFiles) {
        fileList.append(new ProjectExplorer::FileNode(prjDir.absolutePath() + QDir::separator() + file, ProjectExplorer::QMLType, false));
    }
    for (const QString &file : uiFiles) {
        fileList.append(new ProjectExplorer::FileNode(file, ProjectExplorer::FormType, false));
    }
    for (const QString &file : qrcFiles) {
        fileList.append(new ProjectExplorer::FileNode(file, ProjectExplorer::ResourceType, false));
    }
    for (const QString &file : resourceFiles) {
        fileList.append(new ProjectExplorer::FileNode(prjDir.absolutePath() + QDir::separator() + file, ProjectExplorer::UnknownFileType, false));
    }

    buildTree(m_rootNode, fileList);

    // We also need to update deployment data and run configuration for targets.
    for (ProjectExplorer::Target *t : targets()) {
        updateDeploymentDataForTarget(t);
        updateRunConfigurationForTarget(t);
    }

    // And, update our code models
    refreshCppCodeModel();
    createUiCodeModelSupport();

    return true;
}

HemeraProjectManager *HemeraProject::projectManager() const
{
    return m_manager;
}

ProjectExplorer::ProjectNode *HemeraProject::rootProjectNode() const
{
    return m_rootNode;
}

bool HemeraProject::setupTarget(ProjectExplorer::Target *t)
{
    t->updateDefaultBuildConfigurations();
    t->updateDefaultDeployConfigurations();
    t->updateDefaultRunConfigurations();

    return true;
}

bool HemeraProject::supportsKit(ProjectExplorer::Kit *k, QString *errorMessage) const
{
    QVariant tcId = k->value(ProjectExplorer::ToolChainKitInformation::id());
    ProjectExplorer::ToolChain *tc = ProjectExplorer::ToolChainManager::findToolChain(tcId.toString());

    if (!tc || tc->type() != QLatin1String(Constants::TOOLCHAIN_TYPE)) {
        if (errorMessage) {
            *errorMessage = tr("Unsupported kit.");
        }
        return false;
    }

    return true;
}

void HemeraProject::updateRunConfigurations()
{
    foreach (ProjectExplorer::Target *t, targets())
        updateRunConfigurationForTarget(t);
}

// TODO Compare with updateDefaultRunConfigurations();
void HemeraProject::updateRunConfigurationForTarget(ProjectExplorer::Target *t)
{
    // We just need to check whether the target's run configuration are ok.
    QList<ProjectExplorer::RunConfiguration*> configurations = t->runConfigurations();
    for (ProjectExplorer::RunConfiguration *configuration : configurations) {
        HemeraRunConfiguration *hrc = qobject_cast<HemeraRunConfiguration*>(configuration);
        if (!hrc) {
            qDebug() << "Found an incompatible run configuration!!" << configuration;
            // Not compatible!
            t->removeRunConfiguration(configuration);
            continue;
        }

        // Update the application id
        hrc->setApplicationId(m_haManager->applicationId());
    }
}

QString HemeraProject::uiHeaderFile(const QString &uiFile)
{
    QFileInfo fi(uiFile);
    Utils::FileName project = projectDirectory();
    Utils::FileName baseDirectory = Utils::FileName::fromString(fi.absolutePath());

    while (baseDirectory.isChildOf(project)) {
        Utils::FileName hemeraListsTxt = baseDirectory;
        hemeraListsTxt.appendPath(QLatin1String("HemeraLists.txt"));
        if (hemeraListsTxt.toFileInfo().exists())
            break;
        QDir dir(baseDirectory.toString());
        dir.cdUp();
        baseDirectory = Utils::FileName::fromString(dir.absolutePath());
    }

    QDir srcDirRoot = QDir(project.toString());
    QString relativePath = srcDirRoot.relativeFilePath(baseDirectory.toString());
    QDir buildDir = QDir(activeTarget()->activeBuildConfiguration()->buildDirectory().toString());
    QString uiHeaderFilePath = buildDir.absoluteFilePath(relativePath);
    uiHeaderFilePath += QLatin1String("/ui_");
    uiHeaderFilePath += fi.completeBaseName();
    uiHeaderFilePath += QLatin1String(".h");

    return QDir::cleanPath(uiHeaderFilePath);
}

void HemeraProject::updateDeploymentDataForTarget(ProjectExplorer::Target *target)
{
    qDebug() << "Setting deployment data!";
    QDir sourceDir;
    sourceDir.setPath(target->project()->projectDirectory().toString());

    ProjectExplorer::DeploymentData deploymentData;

    // Get our device!
    ProjectExplorer::IDevice::ConstPtr idevice = ProjectExplorer::DeviceKitInformation::device(target->kit());
    HemeraTarget::ConstPtr hemeraTarget = idevice.dynamicCast<const HemeraTarget>();

    QString packageFile = sourceDir.absolutePath() + QDir::separator() + m_haManager->packageNameFor(hemeraTarget->target());
    deploymentData.addFile(packageFile, QString(), ProjectExplorer::DeployableFile::TypeNormal);

    target->setDeploymentData(deploymentData);
}


// HaFile
HaFile::HaFile(HemeraProject *parent, DeveloperMode::HaManager *manager)
    : IDocument(parent),
      m_project(parent),
      m_haManager(manager)
{
    setId("Hemera.ProjectFile");
    setMimeType(QLatin1String(Constants::MIME_TYPE));
    setFilePath(manager->filePath());
}

bool HaFile::save(QString *, const QString &, bool)
{
    return false;
}

QString HaFile::defaultPath() const
{
    return QString();
}

QString HaFile::suggestedFileName() const
{
    return QString();
}

bool HaFile::isModified() const
{
    return false;
}

bool HaFile::isSaveAsAllowed() const
{
    return false;
}

Core::IDocument::ReloadBehavior HaFile::reloadBehavior(ChangeTrigger state, ChangeType type) const
{
    Q_UNUSED(state)
    Q_UNUSED(type)
    return BehaviorSilent;
}

bool HaFile::reload(QString *errorString, ReloadFlag flag, ChangeType type)
{
    Q_UNUSED(errorString)
    Q_UNUSED(flag)
    Q_UNUSED(type)
    return true;
}


HemeraBuildSettingsWidget::HemeraBuildSettingsWidget(HemeraBuildConfiguration *bc) : m_buildConfiguration(0)
{
    QFormLayout *fl = new QFormLayout(this);
    fl->setContentsMargins(20, -1, 0, -1);
    fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    setLayout(fl);

    QPushButton *runCmakeButton = new QPushButton(tr("Run hemera"));
    connect(runCmakeButton, SIGNAL(clicked()), this, SLOT(runHemera()));
    fl->addRow(tr("Reconfigure project:"), runCmakeButton);

    m_pathLineEdit = new QLineEdit(this);
    m_pathLineEdit->setReadOnly(true);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addWidget(m_pathLineEdit);

    m_changeButton = new QPushButton(this);
    m_changeButton->setText(tr("&Change"));
    connect(m_changeButton, SIGNAL(clicked()), this, SLOT(openChangeBuildDirectoryDialog()));
    hbox->addWidget(m_changeButton);

    fl->addRow(tr("Build directory:"), hbox);

    m_buildConfiguration = bc;
    m_pathLineEdit->setText(m_buildConfiguration->rawBuildDirectory().toString());
    if (m_buildConfiguration->buildDirectory() == bc->target()->project()->projectDirectory())
        m_changeButton->setEnabled(false);
    else
        m_changeButton->setEnabled(true);

    setDisplayName(tr("Hemera"));
}

void HemeraBuildSettingsWidget::openChangeBuildDirectoryDialog()
{
    HemeraProject *project = static_cast<HemeraProject *>(m_buildConfiguration->target()->project());
    HemeraBuildInfo info(m_buildConfiguration);
    HemeraOpenProjectWizard copw(project->projectManager(), HemeraOpenProjectWizard::ChangeDirectory, &info);

    if (copw.exec() == QDialog::Accepted) {
        project->changeBuildDirectory(m_buildConfiguration, copw.buildDirectory());
        m_pathLineEdit->setText(m_buildConfiguration->rawBuildDirectory().toString());
    }
}

void HemeraBuildSettingsWidget::runHemera()
{
    if (!ProjectExplorer::ProjectExplorerPlugin::instance()->saveModifiedFiles())
        return;
    HemeraProject *project = static_cast<HemeraProject *>(m_buildConfiguration->target()->project());
    HemeraBuildInfo info(m_buildConfiguration);
    HemeraOpenProjectWizard copw(project->projectManager(),
                                HemeraOpenProjectWizard::WantToUpdate, &info);
    if (copw.exec() == QDialog::Accepted)
        project->parseHaFile();
}

void HemeraBuildTarget::clear()
{
    packageName.clear();
    workingDirectory.clear();
    title.clear();
    library = false;
}

} // namespace Internal
} // namespace Hemera
