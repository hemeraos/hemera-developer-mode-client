#ifndef HEMERA_DEVELOPERMODE_TARGET_P_H
#define HEMERA_DEVELOPERMODE_TARGET_P_H

#include <hemeradevelopermodetarget.h>

#include <hemeradevelopermodecontroller.h>
#include <hemeradevelopermodeoperation.h>

#include "hemeradevelopermodeglobalobjects_p.h"

#include <QtCore/QHash>
#include <QtCore/QPointer>

class QTimer;

namespace Hemera {
namespace DeveloperMode {

class TargetPrivate
{
public:
    TargetPrivate(Target *q) : q_ptr(q), transport(Q_NULLPTR), nam(GlobalObjects::instance()->networkAccessManager()), isOnline(false) {}

    void setDeviceInfo(const QByteArray &infoData = QByteArray(), bool cached = false);
    QByteArray cachedDeviceInfo();
    void cacheDeviceInfo(const QByteArray &infoData);
    void restoreCachedDeviceInfo();

    void setBuildEnvironmentInfo(const QByteArray &infoData = QByteArray(), bool cached = false);
    QByteArray cachedBuildEnvironmentInfo();
    void cacheBuildEnvironmentInfo(const QByteArray &infoData);
    void restoreCachedBuildEnvironmentInfo();

    Q_DECLARE_PUBLIC(Target)
    Target * const q_ptr;

    QString name;
    Transport *transport;
    QNetworkAccessManager *nam;

    QHash< QString, Star* > starObjects;
    Controller::Ptr developerModeController;

    bool discovered;
    bool isOnline;

    QString typeName;

    QString applianceName;
    QString targetName;
    QString id;
    QString applianceId;
    QStringList installedApps;
    bool isProductionBoard;
    bool hasBuildEnvironment;
    QStringList stars;

    QByteArray authToken;

    QString architecture;
    QString hemeraRelease;
    QStringList cpuFlags;
    int cpuFrequency;
    int availableCores;
    int totalMemory;

    QStringList buildArchitectures;
    QString defaultBuildArchitecture;

    void requestDeveloperModeDevice();
    void releaseDeveloperModeDevice();

    // Q_PRIVATE_SLOTS
    void init();
};

class UpdateLocalCacheOperation : public Hemera::DeveloperMode::Operation
{
    Q_OBJECT
public:
    explicit UpdateLocalCacheOperation(Target::Ptr target, QObject* parent = nullptr);
    virtual ~UpdateLocalCacheOperation();

protected:
    virtual void startImpl() override;

    void setThingsToDo(int things);
    void oneThingLessToDo();

private:
    Target::Ptr m_target;

    int m_totalThings;
    int m_leftThings;
};

class EnsureOnlineOperation : public Hemera::DeveloperMode::Operation
{
    Q_OBJECT
public:
    explicit EnsureOnlineOperation(const Target::Ptr &target, int timeout = 5000, QObject *parent = nullptr);
    virtual ~EnsureOnlineOperation();

protected:
    virtual void startImpl() override;

private Q_SLOTS:
    void onOnlineChanged();

private:
    Target::Ptr m_target;
    QTimer *m_timeoutTimer;
};

class EnsureDeveloperModeControllerOperation : public Hemera::DeveloperMode::Operation
{
    Q_OBJECT
public:
    explicit EnsureDeveloperModeControllerOperation(const Target::Ptr &target, int timeout = 5000, QObject *parent = nullptr);
    virtual ~EnsureDeveloperModeControllerOperation();

protected:
    virtual void startImpl() override;

private Q_SLOTS:
    void onDeveloperModeControllerChanged();

private:
    Target::Ptr m_target;
    QTimer *m_timeoutTimer;
};

}
}

#endif // HEMERA_DEVELOPERMODE_TARGET_H
