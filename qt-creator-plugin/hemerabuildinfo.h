#ifndef HEMERABUILDINFO_H
#define HEMERABUILDINFO_H

#include "hemerabuildconfiguration.h"
#include "hemeraconstants.h"
#include <projectexplorer/buildinfo.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/target.h>
#include <projectexplorer/project.h>
#include <utils/environment.h>
#include <utils/qtcassert.h>

namespace Hemera {

class HemeraBuildInfo : public ProjectExplorer::BuildInfo
{
public:
    HemeraBuildInfo(const ProjectExplorer::IBuildConfigurationFactory *f)
        : ProjectExplorer::BuildInfo(f)
    {
    }

    HemeraBuildInfo(const Internal::HemeraBuildConfiguration *bc)
        : ProjectExplorer::BuildInfo(ProjectExplorer::IBuildConfigurationFactory::find(bc->target()))
    {
        displayName = bc->displayName();
        buildDirectory = bc->buildDirectory();
        kitId = bc->target()->kit()->id();
        environment = bc->environment();

        QTC_ASSERT(bc->target()->project(), return);
        sourceDirectory = bc->target()->project()->projectDirectory().toString();
    }

    Utils::Environment environment;
    QString sourceDirectory;
};

} // namespace Hemera

#endif // HEMERABUILDINFO_H
