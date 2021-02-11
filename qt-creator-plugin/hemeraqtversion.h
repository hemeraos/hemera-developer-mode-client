#ifndef HEMERA_INTERNAL_HEMERAQTVERSION_H
#define HEMERA_INTERNAL_HEMERAQTVERSION_H

#include <qtsupport/baseqtversion.h>
#include <qtsupport/qtversionfactory.h>

#include <hemeradevelopermodetarget.h>

namespace Hemera {
namespace DeveloperMode {
class Target;
}
namespace Internal {

class HemeraQtVersion : public QtSupport::BaseQtVersion
{
    Q_DECLARE_TR_FUNCTIONS(Hemera::Internal::HemeraQtVersion)
public:
    HemeraQtVersion();
    HemeraQtVersion(const Utils::FileName &path, const DeveloperMode::Target::Ptr &nativeTarget, bool isAutodetected = true, const QString &autodetectionSource = QString());

    QString targetName() const;

    HemeraQtVersion *clone() const override;

    virtual ~HemeraQtVersion();

    QString type() const override;

    QStringList warningReason() const override;

    QList<ProjectExplorer::Abi> detectQtAbis() const override;

    QString description() const override;

    QString platformName() const override;
    QString platformDisplayName() const override;

    virtual bool supportsShadowBuilds() const override { return true; }

    virtual QList<ProjectExplorer::Task> validateKit(const ProjectExplorer::Kit *k) override;

    QVariantMap toMap() const override;
    void fromMap(const QVariantMap &data) override;
    void addToEnvironment(const ProjectExplorer::Kit *k, Utils::Environment &env) const override;
    Utils::Environment qmakeRunEnvironment() const override;

    // We override this because we don't give a fuck
    Utils::FileName mkspecPath() const;

protected:
    QList<ProjectExplorer::Task> reportIssuesImpl(const QString &proFile,
                                                  const QString &buildDir) const override;

    Core::FeatureSet availableFeatures() const override;

    //virtual void parseMkSpec(ProFileEvaluator *) const override;

private:
    void populateAbiFromArchitecture();

    QString m_targetName;
    QString m_targetArchitecture;
    QList<ProjectExplorer::Abi> m_abis;
};

class HemeraQtVersionFactory : public QtSupport::QtVersionFactory
{
    Q_DECLARE_TR_FUNCTIONS(Hemera::Internal::HemeraQtVersionFactory)
public:
    explicit HemeraQtVersionFactory(QObject *parent = 0);
    virtual ~HemeraQtVersionFactory();

    virtual bool canRestore(const QString &type) override final;
    virtual QtSupport::BaseQtVersion *restore(const QString &type, const QVariantMap &data) override final;

    virtual int priority() const override final;
    virtual QtSupport::BaseQtVersion *create(const Utils::FileName &qmakePath, ProFileEvaluator *evaluator,
                                             bool isAutoDetected = false, const QString &autoDetectionSource = QString()) override final;
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERA_INTERNAL_HEMERAQTVERSION_H
