#include "hemerabuildconfiguration.h"

#include "hemerabuildinfo.h"
#include "hemeradevice.h"
#include "hemeraemulator.h"
#include "hemeraopenprojectwizard.h"
#include "hemeraproject.h"
#include "hemeraconstants.h"
#include "hemerakitinformation.h"
#include "hsdkstep.h"

#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>

#include <utils/qtcassert.h>

#include <QInputDialog>

namespace Hemera {
namespace Internal {

HemeraBuildConfiguration::HemeraBuildConfiguration(ProjectExplorer::Target *parent)
    : BuildConfiguration(parent, Core::Id(Constants::BUILD_CONFIG_ID))
{
}

HemeraBuildConfiguration::HemeraBuildConfiguration(ProjectExplorer::Target *parent, HemeraBuildConfiguration *source)
    : BuildConfiguration(parent, source)
{
    Q_ASSERT(parent);
    cloneSteps(source);
}

HemeraBuildConfiguration::~HemeraBuildConfiguration()
{
}

ProjectExplorer::NamedWidget *HemeraBuildConfiguration::createConfigWidget()
{
    return new HemeraBuildSettingsWidget(this);
}

bool HemeraBuildConfiguration::fromMap(const QVariantMap &map)
{
    return BuildConfiguration::fromMap(map);
}

QVariantMap HemeraBuildConfiguration::toMap() const
{
    return ProjectExplorer::BuildConfiguration::toMap();
}

/*!
  \class HemeraBuildConfigurationFactory
*/

HemeraBuildConfigurationFactory::HemeraBuildConfigurationFactory(QObject *parent) :
    ProjectExplorer::IBuildConfigurationFactory(parent)
{
}

HemeraBuildConfigurationFactory::~HemeraBuildConfigurationFactory()
{
}

int HemeraBuildConfigurationFactory::priority(const ProjectExplorer::Target *parent) const
{
    return canHandle(parent) ? 0 : -1;
}

QList<ProjectExplorer::BuildInfo *> HemeraBuildConfigurationFactory::availableBuilds(const ProjectExplorer::Target *parent) const
{
    return createBuildInfoList(parent->kit(), parent->project()->projectDirectory().toString());
}

int HemeraBuildConfigurationFactory::priority(const ProjectExplorer::Kit *k, const QString &projectPath) const
{
    return (k && Core::MimeDatabase::findByFile(QFileInfo(projectPath))
            .matchesType(QLatin1String(Constants::MIME_TYPE))) ? 0 : -1;
}

QList<ProjectExplorer::BuildInfo *> HemeraBuildConfigurationFactory::availableSetups(const ProjectExplorer::Kit *k, const QString &projectPath) const
{
    return createBuildInfoList(k, ProjectExplorer::Project::projectDirectory(Utils::FileName::fromString(projectPath)).toString());
}

ProjectExplorer::BuildConfiguration *HemeraBuildConfigurationFactory::create(ProjectExplorer::Target *parent, const ProjectExplorer::BuildInfo *info) const
{
    QTC_ASSERT(info->factory() == this,            return 0);
    QTC_ASSERT(info->kitId == parent->kit()->id(), return 0);
    QTC_ASSERT(!info->displayName.isEmpty(),       return 0);

    qDebug() << "HemeraBuildConfigurationFactory::create" << info->displayName;

    HemeraBuildInfo infoCopy(*static_cast<const HemeraBuildInfo *>(info));
    HemeraProject *project = static_cast<HemeraProject *>(parent->project());

    HemeraBuildConfiguration *bc = new HemeraBuildConfiguration(parent);
    bc->setDisplayName(infoCopy.displayName);
    bc->setDefaultDisplayName(infoCopy.displayName);
    bc->setBuildDirectory(infoCopy.buildDirectory);

    // Find out about our device
    ProjectExplorer::IDevice::ConstPtr idevice = ProjectExplorer::DeviceKitInformation::device(parent->kit());
    HemeraTarget::ConstPtr hemeraTarget = idevice.dynamicCast<const HemeraTarget>();
    if (hemeraTarget.isNull()) {
        return 0;
    }

    // build steps
    ProjectExplorer::BuildStepList *buildSteps = bc->stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);

    if (buildSteps) {
        HsdkStep *configureStep = new HsdkStep(buildSteps);
        configureStep->setAdditionalArguments(QStringList() << QLatin1String("configure"));
        HsdkStep *buildProjectStep = new HsdkStep(buildSteps);
        buildProjectStep->setAdditionalArguments(QStringList() << QLatin1String("full-build") << hemeraTarget->hsdkArguments());

        buildSteps->insertStep(0, configureStep);
        buildSteps->insertStep(1, buildProjectStep);

        // Default to all
        if (project->hasBuildTarget(QLatin1String("all"))) {
            configureStep->setBuildTarget(QLatin1String("all"), true);
        }
    } else {
        qWarning() << "HemeraBuildConfigurationFactory::create: Build steps is empty!";
    }

    // clean steps
    ProjectExplorer::BuildStepList *cleanSteps = bc->stepList(ProjectExplorer::Constants::BUILDSTEPS_CLEAN);

    if (cleanSteps) {
        HsdkStep *makeCleanStep = new HsdkStep(cleanSteps);
        makeCleanStep->setAdditionalArguments(QStringList() << QLatin1String("wipe"));
        makeCleanStep->setClean(true);

        cleanSteps->insertStep(0, makeCleanStep);
    } else {
        qWarning() << "HemeraBuildConfigurationFactory::create: Clean steps is empty!";
    }

    return bc;
}

bool HemeraBuildConfigurationFactory::canClone(const ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source) const
{
    if (!canHandle(parent))
        return false;
    return source->id() == Constants::BUILD_CONFIG_ID;
}

HemeraBuildConfiguration *HemeraBuildConfigurationFactory::clone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source)
{
    if (!canClone(parent, source))
        return 0;
    HemeraBuildConfiguration *old = static_cast<HemeraBuildConfiguration *>(source);
    return new HemeraBuildConfiguration(parent, old);
}

bool HemeraBuildConfigurationFactory::canRestore(const ProjectExplorer::Target *parent, const QVariantMap &map) const
{
    if (!canHandle(parent))
        return false;
    return ProjectExplorer::idFromMap(map) == Constants::BUILD_CONFIG_ID;
}

HemeraBuildConfiguration *HemeraBuildConfigurationFactory::restore(ProjectExplorer::Target *parent, const QVariantMap &map)
{
    qDebug() << "Restore attempt!";
    if (!canRestore(parent, map))
        return 0;
    qDebug() << "Restoring...";
    HemeraBuildConfiguration *bc = new HemeraBuildConfiguration(parent);
    if (bc->fromMap(map))
        return bc;
    delete bc;
    return 0;
}

bool HemeraBuildConfigurationFactory::canHandle(const ProjectExplorer::Target *t) const
{
    QTC_ASSERT(t, return false);
    if (!t->project()->supportsKit(t->kit()))
        return false;
    return qobject_cast<HemeraProject *>(t->project());
}

QList<ProjectExplorer::BuildInfo *> HemeraBuildConfigurationFactory::createBuildInfoList(const ProjectExplorer::Kit *k, const QString &dirPath) const
{
    QList<ProjectExplorer::BuildInfo *> result;

    QStringList availableTypes = QStringList() << QStringLiteral("Debug") << QStringLiteral("Release");
    for (const QString &type : availableTypes) {
        HemeraBuildInfo *info = new HemeraBuildInfo(this);
        info->typeName            = type;
        info->displayName         = tr("Package (%1)").arg(type);
        info->kitId               = k->id();
        info->environment         = Utils::Environment::systemEnvironment();
        info->sourceDirectory     = dirPath;
        info->buildDirectory      = Utils::FileName::fromString(dirPath);
        info->supportsShadowBuild = false;

        result << info;
    }

    return result;
}

ProjectExplorer::BuildConfiguration::BuildType HemeraBuildConfiguration::buildType() const
{
    QString hemeraBuildType;
    QFile hemeraCache(buildDirectory().toString() + QLatin1String("/HemeraCache.txt"));
    if (hemeraCache.open(QIODevice::ReadOnly)) {
        while (!hemeraCache.atEnd()) {
            QByteArray line = hemeraCache.readLine();
            if (line.startsWith("HEMERA_BUILD_TYPE")) {
                if (int pos = line.indexOf('='))
                    hemeraBuildType = QString::fromLocal8Bit(line.mid(pos + 1).trimmed());
                break;
            }
        }
        hemeraCache.close();
    }

    // Cover all common Hemera build types
    if (hemeraBuildType.compare(QLatin1String("Release"), Qt::CaseInsensitive) == 0
        || hemeraBuildType.compare(QLatin1String("MinSizeRel"), Qt::CaseInsensitive) == 0)
    {
        return Release;
    } else if (hemeraBuildType.compare(QLatin1String("Debug"), Qt::CaseInsensitive) == 0
               || hemeraBuildType.compare(QLatin1String("DebugFull"), Qt::CaseInsensitive) == 0
               || hemeraBuildType.compare(QLatin1String("RelWithDebInfo"), Qt::CaseInsensitive) == 0)
    {
        return Debug;
    }

    return Unknown;
}

} // namespace Internal
} // namespace Hemera
