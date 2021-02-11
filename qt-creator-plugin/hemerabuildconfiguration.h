#ifndef HEMERABUILDCONFIGURATION_H
#define HEMERABUILDCONFIGURATION_H

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/abi.h>

namespace ProjectEFxplorer {
class ToolChain;
}

namespace Hemera {
class HemeraBuildInfo;

namespace Internal {
class HemeraProject;

class HemeraBuildConfigurationFactory;

class HemeraBuildConfiguration : public ProjectExplorer::BuildConfiguration
{
    Q_OBJECT
    friend class HemeraBuildConfigurationFactory;

public:
    HemeraBuildConfiguration(ProjectExplorer::Target *parent);
    ~HemeraBuildConfiguration();

    ProjectExplorer::NamedWidget *createConfigWidget();

    QVariantMap toMap() const;

    BuildType buildType() const;

protected:
    HemeraBuildConfiguration(ProjectExplorer::Target *parent, HemeraBuildConfiguration *source);
    bool fromMap(const QVariantMap &map);

    friend class HemeraProject;
};

class HemeraBuildConfigurationFactory : public ProjectExplorer::IBuildConfigurationFactory
{
    Q_OBJECT

public:
    HemeraBuildConfigurationFactory(QObject *parent = 0);
    ~HemeraBuildConfigurationFactory();

    int priority(const ProjectExplorer::Target *parent) const;
    QList<ProjectExplorer::BuildInfo *> availableBuilds(const ProjectExplorer::Target *parent) const;
    int priority(const ProjectExplorer::Kit *k, const QString &projectPath) const;
    QList<ProjectExplorer::BuildInfo *> availableSetups(const ProjectExplorer::Kit *k,
                                                        const QString &projectPath) const;
    ProjectExplorer::BuildConfiguration *create(ProjectExplorer::Target *parent,
                                                const ProjectExplorer::BuildInfo *info) const;

    bool canClone(const ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source) const;
    HemeraBuildConfiguration *clone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source);
    bool canRestore(const ProjectExplorer::Target *parent, const QVariantMap &map) const;
    HemeraBuildConfiguration *restore(ProjectExplorer::Target *parent, const QVariantMap &map);

private:
    bool canHandle(const ProjectExplorer::Target *t) const;
    QList<ProjectExplorer::BuildInfo *> createBuildInfoList(const ProjectExplorer::Kit *k, const QString &dirPath) const;
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERABUILDCONFIGURATION_H
