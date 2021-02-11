#ifndef HEMERA_INTERNAL_HEMERAKITINFORMATION_H
#define HEMERA_INTERNAL_HEMERAKITINFORMATION_H

#include <projectexplorer/kitinformation.h>

namespace Hemera {
namespace Internal {

class HemeraKitInformation : public ProjectExplorer::KitInformation
{
    Q_DECLARE_TR_FUNCTIONS(Hemera::HemeraKitInformation)
public:
    explicit HemeraKitInformation();
    virtual ~HemeraKitInformation();

    QVariant defaultValue(ProjectExplorer::Kit *kit) const;
    QList<ProjectExplorer::Task> validate(const ProjectExplorer::Kit *kit) const;
    ItemList toUserOutput(const ProjectExplorer::Kit *kit) const;
    ProjectExplorer::KitConfigWidget *createConfigWidget(ProjectExplorer::Kit *kit) const;
    void addToEnvironment(const ProjectExplorer::Kit *kit, Utils::Environment &env) const;

    virtual QSet<QString> availablePlatforms(const ProjectExplorer::Kit *k) const override;
    virtual QString displayNameForPlatform(const ProjectExplorer::Kit *k, const QString &platform) const override;
    virtual Core::FeatureSet availableFeatures(const ProjectExplorer::Kit *k) const override;

    static void setTargetName(ProjectExplorer::Kit *kit, const QString& targetName);
    static QString targetName(const ProjectExplorer::Kit *kit);

    static ProjectExplorer::KitMatcher hemeraKitMatcher();
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERA_INTERNAL_HEMERAKITINFORMATION_H
