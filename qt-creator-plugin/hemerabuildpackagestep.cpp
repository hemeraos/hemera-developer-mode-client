#include "hemerabuildpackagestep.h"

#include <projectexplorer/target.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/kitinformation.h>

namespace Hemera {
namespace Internal {

HemeraBuildPackageStep::HemeraBuildPackageStep(ProjectExplorer::BuildStepList *bsl, Core::Id id)
    : BuildStep(bsl, id)
{
}

HemeraBuildPackageStep::HemeraBuildPackageStep(ProjectExplorer::BuildStepList *bsl, ProjectExplorer::BuildStep *bs)
    : BuildStep(bsl, bs)
{
}

HemeraBuildPackageStep::~HemeraBuildPackageStep()
{
}

HemeraBuildConfiguration *HemeraBuildPackageStep::targetsActiveBuildConfiguration() const
{
    return static_cast<HemeraBuildConfiguration *>(target()->activeBuildConfiguration());
}

HemeraBuildConfiguration *HemeraBuildPackageStep::hemeraBuildConfiguration() const
{
    return static_cast<HemeraBuildConfiguration *>(buildConfiguration());
}

bool HemeraBuildPackageStep::init()
{
    HemeraBuildConfiguration *bc = hemeraBuildConfiguration();
    if (!bc) {
        bc = targetsActiveBuildConfiguration();
    }

    if (!bc) {
        emit addTask(ProjectExplorer::Task::buildConfigurationMissingTask());
    }

    ProjectExplorer::ToolChain *tc = ProjectExplorer::ToolChainKitInformation::toolChain(target()->kit());
    if (!tc)
        emit addTask(ProjectExplorer::Task::compilerMissingTask());

    if (!bc || !tc) {
        emit addOutput(tr("Configuration is faulty. Check the Issues view for details."),
                       BuildStep::MessageOutput);
        return false;
    }
}

void HemeraBuildPackageStep::run(QFutureInterface<bool> &fi)
{
    // We don't need this interface, we're running asynchronously
    Q_UNUSED(fi);
}

ProjectExplorer::BuildStepConfigWidget *HemeraBuildPackageStep::createConfigWidget()
{

}

} // namespace Internal
} // namespace Hemera

