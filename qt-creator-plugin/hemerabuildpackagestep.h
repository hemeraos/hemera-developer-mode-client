#ifndef HEMERA_INTERNAL_HEMERABUILDPACKAGESTEP_H
#define HEMERA_INTERNAL_HEMERABUILDPACKAGESTEP_H

#include <projectexplorer/buildstep.h>

#include <hemerabuildconfiguration.h>

namespace Hemera {
namespace Internal {

class HemeraBuildPackageStep : public ProjectExplorer::BuildStep
{
    Q_OBJECT
public:
    explicit HemeraBuildPackageStep(ProjectExplorer::BuildStepList *bsl, Core::Id id);
    explicit HemeraBuildPackageStep(ProjectExplorer::BuildStepList *bsl, ProjectExplorer::BuildStep *bs);
    virtual ~HemeraBuildPackageStep();

    virtual bool init() override;

    virtual void run(QFutureInterface<bool> &fi) override;

    virtual ProjectExplorer::BuildStepConfigWidget *createConfigWidget() override;

    inline virtual bool immutable() const override { return true; }
    inline virtual bool runInGuiThread() const override { return true; } // We're async.

protected:
    // Utils
    HemeraBuildConfiguration *hemeraBuildConfiguration() const;
    HemeraBuildConfiguration *targetsActiveBuildConfiguration() const;

private:

};

} // namespace Internal
} // namespace Hemera

#endif // HEMERA_INTERNAL_HEMERABUILDPACKAGESTEP_H
