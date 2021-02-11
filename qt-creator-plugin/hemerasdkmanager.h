#ifndef HEMERA_INTERNAL_HEMERASDKMANAGER_H
#define HEMERA_INTERNAL_HEMERASDKMANAGER_H

#include <QtCore/QObject>

#include <coreplugin/id.h>
#include <utils/environment.h>

#include <hemeratarget.h>

namespace Utils {
class PersistentSettingsWriter;
class FileName;
}

namespace ProjectExplorer {
class Kit;
class Project;
}

namespace Hemera {
namespace Internal {

class HemeraQtVersion;
class HemeraToolChain;

class HemeraSDKManager : public QObject
{
    Q_OBJECT
public:
    static HemeraSDKManager *instance();
    static QString sdkToolsDirectory();
    static QString globalSdkToolsDirectory();
    static bool isHemeraKit(const ProjectExplorer::Kit *kit);
    static QString targetNameForKit(const ProjectExplorer::Kit *kit);
    static QList<ProjectExplorer::Kit *> kitsForTarget(const QString &targetName);
    static bool hasHemeraDevice(ProjectExplorer::Kit *kit);
    static bool validateKit(const ProjectExplorer::Kit* kit);

    virtual ~HemeraSDKManager();

    QList<ProjectExplorer::Kit*> hemeraKits() const;

signals:
    void sdksUpdated();
    void initialized();

private slots:
    void initialize();
    void updateDevices();
    void onDeviceRemoved(const Core::Id &id);

private:
    HemeraSDKManager();
    void restore();
    QList<HemeraToolChain*> hemeraToolChains() const;
    QList<HemeraQtVersion*> hemeraQtVersions() const;
    const Utils::FileName& checkInstallLocation(const Utils::FileName &l, const Utils::FileName &g);
private:
    static HemeraSDKManager *m_instance;
    bool m_intialized;
    Utils::PersistentSettingsWriter *m_writer;
    QString m_installDir;
    bool m_reinstall;

    QHash<Core::Id, HemeraTarget::ConstPtr> m_hemeraDevicesCache;

    friend class HemeraPlugin;

public:
    static bool verbose;
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERA_INTERNAL_HEMERASDKMANAGER_H
