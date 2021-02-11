#include "hemeradeploystep.h"

#include "hemeraconstants.h"
#include "hemeradeployconfiguration.h"

#include <hemeradevelopermodecontroller.h>
#include <hemeradevelopermodedeployoperation.h>
#include <transports/hemeradevelopermodetransport.h>

#include <QtCore/QTimer>

#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/deploymentdata.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>
#include <projectexplorer/task.h>

namespace Hemera {
namespace Internal {

HemeraDeployStep::HemeraDeployStep(ProjectExplorer::BuildStepList *bsl)
    : BuildStep(bsl, Constants::DEPLOY_STEP_ID)
{
}

HemeraDeployStep::HemeraDeployStep(ProjectExplorer::BuildStepList *bsl, HemeraDeployStep *other)
    : BuildStep(bsl, other)
{
}

HemeraDeployStep::~HemeraDeployStep()
{
}

bool HemeraDeployStep::fromMap(const QVariantMap &map)
{
    if (!BuildStep::fromMap(map)) {
        return false;
    }
    return true;
}

QVariantMap HemeraDeployStep::toMap() const
{
    return BuildStep::toMap();
}

bool HemeraDeployStep::init()
{
    // We have to verify our target is good first.
    DeveloperMode::Target::Ptr nativeTarget = deployConfiguration()->nativeTarget();
    if (nativeTarget.isNull()) {
        return false;
    }

    return true;
}

void HemeraDeployStep::run(QFutureInterface<bool> &fi)
{
    m_future = fi;

    emit addOutput(tr("Waiting for target... "), MessageOutput, DontAppendNewline);

    // Let's connect our target first.
    connect(deployConfiguration()->nativeTarget()->ensureDeveloperModeController(), &DeveloperMode::Operation::finished,
            this, &HemeraDeployStep::onDeveloperModeControllerEnsured);
}

void HemeraDeployStep::onDeveloperModeControllerEnsured(DeveloperMode::Operation *operation)
{
    if (operation->isError()) {
        // Ouch...
        QString message = QStringLiteral("%1: %2.").arg(operation->errorName(), operation->errorMessage());
        Q_EMIT addOutput(message, ErrorMessageOutput);
        Q_EMIT addTask(ProjectExplorer::Task(ProjectExplorer::Task::Error, message, Utils::FileName(), -1,
                          ProjectExplorer::Constants::TASK_CATEGORY_DEPLOYMENT));
        message = tr("Target %1 Developer Mode Controller could not be retrieved.").arg(deployConfiguration()->nativeTarget()->name());
        Q_EMIT addOutput(message, ErrorMessageOutput);
        Q_EMIT addTask(ProjectExplorer::Task(ProjectExplorer::Task::Error, message, Utils::FileName(), -1,
                          ProjectExplorer::Constants::TASK_CATEGORY_DEPLOYMENT));
        handleFinished();
        return;
    }

    // We're set. Deployment can start.
    emit addOutput(tr("online."), MessageOutput);

    // Verify we have our package
    if (target()->deploymentData().allFiles().size() != 1) {
        qDebug() << target()->deploymentData().allFiles().size();
        QString message = tr("Deployment data is corrupted, more than one package is a candidate for deployment. This is an internal error.");
        Q_EMIT addOutput(message, ErrorMessageOutput);
        Q_EMIT addTask(ProjectExplorer::Task(ProjectExplorer::Task::Error, message, Utils::FileName(), -1,
                          ProjectExplorer::Constants::TASK_CATEGORY_DEPLOYMENT));
        handleFinished();
        return;
    }
    ProjectExplorer::DeployableFile deployableFile = target()->deploymentData().allFiles().first();
    QString packagePath = deployableFile.localFilePath().toString();
    if (!QFile::exists(packagePath)) {
        QString message = tr("Deployment data refers to %1, but the file was not found.").arg(packagePath);
        Q_EMIT addOutput(message, ErrorMessageOutput);
        Q_EMIT addTask(ProjectExplorer::Task(ProjectExplorer::Task::Error, message, Utils::FileName(), -1,
                          ProjectExplorer::Constants::TASK_CATEGORY_DEPLOYMENT));
        handleFinished();
        return;
    }

    m_operation = deployConfiguration()->nativeTarget()->developerModeController()->deployPackage(packagePath);
    if (m_operation.isNull()) {
        // Ouch...
        QString message = tr("Could not create Deploy Operation! This is a serious internal error.").arg(deployConfiguration()->nativeTarget()->name());
        Q_EMIT addOutput(message, ErrorMessageOutput);
        Q_EMIT addTask(ProjectExplorer::Task(ProjectExplorer::Task::Error, message, Utils::FileName(), -1,
                          ProjectExplorer::Constants::TASK_CATEGORY_DEPLOYMENT));
        handleFinished();
        return;
    }

    Q_EMIT addOutput(tr("Uploading to target..."), MessageOutput);

    connect(m_operation.data(), &DeveloperMode::Operation::finished, this, &HemeraDeployStep::handleFinished);
    connect(m_operation.data(), &DeveloperMode::DeployOperation::progress, [this] (quint64 bytesUploaded, quint64 totalBytes, quint64) {
        if (bytesUploaded == totalBytes) {
            // Install phase started
            Q_EMIT addOutput(tr("Installing application to target..."), MessageOutput);
        }
    });
}

void HemeraDeployStep::cancel()
{
    if (m_operation.isNull()) {
        return;
    }

    emit addOutput(tr("User requests deployment to stop; cleaning up."), MessageOutput);
}

ProjectExplorer::BuildStepConfigWidget *HemeraDeployStep::createConfigWidget()
{
    return new ProjectExplorer::SimpleBuildStepConfigWidget(this);
}

HemeraDeployConfiguration *HemeraDeployStep::deployConfiguration() const
{
    return qobject_cast<HemeraDeployConfiguration *>(BuildStep::deployConfiguration());
}

void HemeraDeployStep::handleFinished()
{
    bool success = false;

    if (m_operation.isNull()) {
        Q_EMIT addOutput(tr("Deploy step failed."), ErrorMessageOutput);
    } else if (m_operation->isError()) {
        Q_EMIT addOutput(QStringLiteral("%1: %2").arg(m_operation->errorName(), m_operation->errorMessage()), ErrorMessageOutput);
        Q_EMIT addOutput(tr("Deploy step failed."), ErrorMessageOutput);
    } else {
        Q_EMIT addOutput(tr("Deploy step finished."), MessageOutput);
        success = true;
    }

    qDebug() << "Handle finished:" << success;

    m_future.reportResult(success);
    Q_EMIT finished();
}


////// Factory
///

HemeraDeployStepFactory::HemeraDeployStepFactory(QObject *parent) :
    ProjectExplorer::IBuildStepFactory(parent)
{
}

HemeraDeployStepFactory::~HemeraDeployStepFactory()
{
}

bool HemeraDeployStepFactory::canCreate(ProjectExplorer::BuildStepList *parent, const Core::Id id) const
{
    if (parent->target()->project()->id() == Constants::PROJECT_ID)
        return id == Hemera::Constants::DEPLOY_STEP_ID;
    return false;
}

ProjectExplorer::BuildStep *HemeraDeployStepFactory::create(ProjectExplorer::BuildStepList *parent, const Core::Id id)
{
    if (!canCreate(parent, id))
        return 0;
    return new HemeraDeployStep(parent);
}

bool HemeraDeployStepFactory::canClone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *source) const
{
    return canCreate(parent, source->id());
}

ProjectExplorer::BuildStep *HemeraDeployStepFactory::clone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *source)
{
    if (!canClone(parent, source))
        return 0;
    return new HemeraDeployStep(parent, static_cast<HemeraDeployStep *>(source));
}

bool HemeraDeployStepFactory::canRestore(ProjectExplorer::BuildStepList *parent, const QVariantMap &map) const
{
    return canCreate(parent, ProjectExplorer::idFromMap(map));
}

ProjectExplorer::BuildStep *HemeraDeployStepFactory::restore(ProjectExplorer::BuildStepList *parent, const QVariantMap &map)
{
    if (!canRestore(parent, map))
        return 0;
    HemeraDeployStep *bs(new HemeraDeployStep(parent));
    if (bs->fromMap(map))
        return bs;
    delete bs;
    return 0;
}

QList<Core::Id> HemeraDeployStepFactory::availableCreationIds(ProjectExplorer::BuildStepList *parent) const
{
    if (parent->target()->project()->id() == Constants::PROJECT_ID)
        return QList<Core::Id>() << Core::Id(Hemera::Constants::DEPLOY_STEP_ID);
    return QList<Core::Id>();
}

QString HemeraDeployStepFactory::displayNameForId(const Core::Id id) const
{
    if (id == Hemera::Constants::DEPLOY_STEP_ID)
        return tr("Deploy to Hemera Target");
    return QString();
}

} // namespace Internal
} // namespace Hemera

