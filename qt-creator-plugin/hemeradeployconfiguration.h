#ifndef HEMERA_INTERNAL_HEMERADEPLOYCONFIGURATION_H
#define HEMERA_INTERNAL_HEMERADEPLOYCONFIGURATION_H

#include <projectexplorer/deployconfiguration.h>

#include <hemeradevelopermodetarget.h>

namespace Hemera {
namespace Internal {

class HemeraDeployConfiguration : public ProjectExplorer::DeployConfiguration
{
    Q_OBJECT
    friend class HemeraDeployConfigurationFactory;

public:
    HemeraDeployConfiguration(ProjectExplorer::Target *parent, Core::Id id);

    DeveloperMode::Target::Ptr nativeTarget() const;

protected:
    HemeraDeployConfiguration(ProjectExplorer::Target *parent, ProjectExplorer::DeployConfiguration *source);

private:
    DeveloperMode::Target::Ptr m_nativeTarget;
};

class HemeraDeployConfigurationFactory : public ProjectExplorer::DeployConfigurationFactory
{
    Q_OBJECT

public:
    explicit HemeraDeployConfigurationFactory(QObject *parent = 0);

    bool canCreate(ProjectExplorer::Target *parent, Core::Id id) const;
    ProjectExplorer::DeployConfiguration *create(ProjectExplorer::Target *parent, Core::Id id);
    bool canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const;
    ProjectExplorer::DeployConfiguration *restore(ProjectExplorer::Target *parent, const QVariantMap &map);
    bool canClone(ProjectExplorer::Target *parent, ProjectExplorer::DeployConfiguration *source) const;
    ProjectExplorer::DeployConfiguration *clone(ProjectExplorer::Target *parent, ProjectExplorer::DeployConfiguration *source);

    QList<Core::Id> availableCreationIds(ProjectExplorer::Target *parent) const;
    // used to translate the ids to names to display to the user
    QString displayNameForId(Core::Id id) const;

private:
    bool canHandle(ProjectExplorer::Target *parent) const;
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERA_INTERNAL_HEMERADEPLOYCONFIGURATION_H
