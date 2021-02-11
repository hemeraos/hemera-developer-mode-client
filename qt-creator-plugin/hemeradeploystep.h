#ifndef HEMERA_INTERNAL_HEMERADEPLOYSTEP_H
#define HEMERA_INTERNAL_HEMERADEPLOYSTEP_H

#include <projectexplorer/buildstep.h>

#include <QtCore/QPointer>

namespace Hemera {
namespace DeveloperMode {
class DeployOperation;
class Operation;
}
namespace Internal {

class HemeraDeployConfiguration;

class HemeraDeployStep : public ProjectExplorer::BuildStep
{
    Q_OBJECT

public:
    HemeraDeployStep(ProjectExplorer::BuildStepList *bsl);
    HemeraDeployStep(ProjectExplorer::BuildStepList *bsl, HemeraDeployStep *other);
    virtual ~HemeraDeployStep();

    bool fromMap(const QVariantMap &map) override;
    QVariantMap toMap() const override;

    virtual bool init() override;
    virtual void run(QFutureInterface<bool> &fi) override;
    virtual bool runInGuiThread() const override { return true; }
    virtual void cancel() override;

    ProjectExplorer::BuildStepConfigWidget *createConfigWidget() override;

    HemeraDeployConfiguration *deployConfiguration() const;

private Q_SLOTS:
    void onDeveloperModeControllerEnsured(DeveloperMode::Operation *operation);

    void handleFinished();

private:
    QFutureInterface<bool> m_future;

    QPointer<DeveloperMode::DeployOperation> m_operation;
};

class HemeraDeployStepFactory : public ProjectExplorer::IBuildStepFactory
{
    Q_OBJECT

public:
    explicit HemeraDeployStepFactory(QObject *parent = 0);
    virtual ~HemeraDeployStepFactory();

    bool canCreate(ProjectExplorer::BuildStepList *parent, const Core::Id id) const;
    ProjectExplorer::BuildStep *create(ProjectExplorer::BuildStepList *parent, const Core::Id id);
    bool canClone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *source) const;
    ProjectExplorer::BuildStep *clone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *source);
    bool canRestore(ProjectExplorer::BuildStepList *parent, const QVariantMap &map) const;
    ProjectExplorer::BuildStep *restore(ProjectExplorer::BuildStepList *parent, const QVariantMap &map);

    QList<Core::Id> availableCreationIds(ProjectExplorer::BuildStepList *bc) const;
    QString displayNameForId(const Core::Id id) const;
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERA_INTERNAL_HEMERADEPLOYSTEP_H
