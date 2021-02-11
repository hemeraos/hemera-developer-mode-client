#ifndef HSDKSTEP_H
#define HSDKSTEP_H

#include <projectexplorer/abstractprocessstep.h>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QListWidget;
class QListWidgetItem;
QT_END_NAMESPACE

namespace ProjectExplorer {
class ToolChain;
}

namespace Hemera {
namespace Internal {

class HemeraBuildConfiguration;
class HsdkStepFactory;

class HsdkStep : public ProjectExplorer::AbstractProcessStep
{
    Q_OBJECT
    friend class HsdkStepFactory;

public:
    HsdkStep(ProjectExplorer::BuildStepList *bsl);
    virtual ~HsdkStep();

    HemeraBuildConfiguration *hemeraBuildConfiguration() const;

    virtual bool init();

    virtual void run(QFutureInterface<bool> &fi);

    virtual ProjectExplorer::BuildStepConfigWidget *createConfigWidget();
    virtual bool immutable() const;

    QStringList buildTargets() const;
    bool buildsBuildTarget(const QString &target) const;
    void setBuildTarget(const QString &target, bool on);
    void setBuildTargets(const QStringList &targets);
    void clearBuildTargets();

    QStringList additionalArguments() const;
    void setAdditionalArguments(const QStringList &list);

    QString hsdkCommand(ProjectExplorer::ToolChain *tc, const Utils::Environment &env) const;

    void setClean(bool clean);

    QVariantMap toMap() const;

public slots:
    void activeBuildConfigurationChanged();

private slots:
    void buildTargetsChanged();

signals:
    void hsdkCommandChanged();
    void targetsToBuildChanged();

protected:
    void processStarted();
    void processFinished(int exitCode, QProcess::ExitStatus status);

    HsdkStep(ProjectExplorer::BuildStepList *bsl, HsdkStep *bs);
    HsdkStep(ProjectExplorer::BuildStepList *bsl, const Core::Id id);

    bool fromMap(const QVariantMap &map);

    // For parsing [ 76%]
    virtual void stdOutput(const QString &line);

private:
    void ctor();
    HemeraBuildConfiguration *targetsActiveBuildConfiguration() const;

    bool m_clean;
    QRegExp m_percentProgress;
    QRegExp m_ninjaProgress;
    QString m_ninjaProgressString;
    QStringList m_buildTargets;
    QStringList m_additionalArguments;
    QList<ProjectExplorer::Task> m_tasks;
//    bool m_useNinja;
    HemeraBuildConfiguration *m_activeConfiguration;
};

class HsdkStepConfigWidget :public ProjectExplorer::BuildStepConfigWidget
{
    Q_OBJECT
public:
    HsdkStepConfigWidget(HsdkStep *hsdkStep);
    virtual QString displayName() const;
    virtual QString summaryText() const;
private slots:
    void itemChanged(QListWidgetItem*);
    void additionalArgumentsEdited();
    void updateDetails();
    void buildTargetsChanged();
    void selectedBuildTargetsChanged();

private:
    HsdkStep *m_hsdkStep;
    QListWidget *m_buildTargetsList;
    QLineEdit *m_additionalArguments;
    QString m_summaryText;
};

class HsdkStepFactory : public ProjectExplorer::IBuildStepFactory
{
    Q_OBJECT

public:
    explicit HsdkStepFactory(QObject *parent = 0);
    virtual ~HsdkStepFactory();

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

#endif // HSDKSTEP_H
