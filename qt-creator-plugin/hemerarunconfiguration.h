#ifndef HEMERARUNCONFIGURATION_H
#define HEMERARUNCONFIGURATION_H

#include <projectexplorer/runconfiguration.h>

#include <utils/detailswidget.h>
#include <utils/environment.h>

#include <hemeradevelopermodecontroller.h>
#include <hemeradevelopermodetarget.h>

QT_BEGIN_NAMESPACE
class QComboBox;
QT_END_NAMESPACE

namespace Utils {
class PathChooser;
class DetailsWidget;
}

namespace Hemera {
namespace DeveloperMode {
class Controller;
class Operation;
}
namespace Internal {
namespace Ui {
class HemeraRunConfigurationWidget;
}

class HemeraTarget;

class HemeraRunConfiguration : public ProjectExplorer::RunConfiguration
{
    Q_OBJECT
    friend class HemeraRunConfigurationWidget;
    friend class HemeraRunConfigurationFactory;

public:
    explicit HemeraRunConfiguration(ProjectExplorer::Target *parent, Core::Id id);
    virtual ~HemeraRunConfiguration();

    virtual bool fromMap(const QVariantMap &map) override;
    virtual QVariantMap toMap() const override;

    QString applicationId() const;
    QString star() const;

    virtual QWidget *createConfigurationWidget() override;
    virtual Utils::OutputFormatter *createOutputFormatter() const override;

public Q_SLOTS:
    void setApplicationId(const QString &applicationId);
    void setStar(const QString &star);

private:
    QString m_starName;
    QString m_applicationId;
};

class HemeraRunConfigurationWidget : public Utils::DetailsWidget
{
    Q_OBJECT
public:
    explicit HemeraRunConfigurationWidget(HemeraRunConfiguration *hemeraRunConfiguration, QWidget *parent = 0);
    virtual ~HemeraRunConfigurationWidget();

public Q_SLOTS:
    void updateSummary();

private:
    Ui::HemeraRunConfigurationWidget *ui;

    HemeraRunConfiguration *m_hemeraRunConfiguration;
};

class HemeraRunConfigurationFactory : public ProjectExplorer::IRunConfigurationFactory
{
    Q_OBJECT

public:
    explicit HemeraRunConfigurationFactory(QObject *parent = 0);
    virtual ~HemeraRunConfigurationFactory();

    QList<Core::Id> availableCreationIds(ProjectExplorer::Target *parent, CreationMode mode = UserCreate) const override;

    bool canCreate(ProjectExplorer::Target *parent, Core::Id id) const override;

    bool canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const override;

    bool canClone(ProjectExplorer::Target *parent, ProjectExplorer::RunConfiguration *source) const override;
    ProjectExplorer::RunConfiguration *clone(ProjectExplorer::Target *parent,
                                             ProjectExplorer::RunConfiguration *source) override;

    virtual QString displayNameForId(Core::Id id) const override;

private:
    bool canHandle(const ProjectExplorer::Target *target) const;

    ProjectExplorer::RunConfiguration *doCreate(ProjectExplorer::Target *parent, Core::Id id);
    ProjectExplorer::RunConfiguration *doRestore(ProjectExplorer::Target *parent,
                                                 const QVariantMap &map);
};

class HemeraRunControl : public ProjectExplorer::RunControl
{
    Q_OBJECT

public:
    explicit HemeraRunControl(HemeraRunConfiguration *runConfig);
    virtual ~HemeraRunControl();

    virtual void start() override;
    virtual StopResult stop() override;
    virtual bool isRunning() const override;
    virtual QString displayName() const override;

private Q_SLOTS:
    void onDeveloperModeControllerEnsured(DeveloperMode::Operation *operation);

private:
    bool m_shuttingDown;
    DeveloperMode::Target::Ptr m_nativeTarget;
    DeveloperMode::Controller::Ptr m_controller;
};

class HemeraRunControlFactory : public ProjectExplorer::IRunControlFactory
{
    Q_OBJECT

public:
    explicit HemeraRunControlFactory(QObject *parent = 0);

    virtual bool canRun(ProjectExplorer::RunConfiguration *runConfiguration,
                        ProjectExplorer::RunMode mode) const override;
    virtual ProjectExplorer::RunControl *create(ProjectExplorer::RunConfiguration *runConfiguration,
                                                ProjectExplorer::RunMode mode,
                                                QString *errorMessage) override;
};


} // namespace Internal
} // namespace Hemera

#endif // HEMERARUNCONFIGURATION_H
