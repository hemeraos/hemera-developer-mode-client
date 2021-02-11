/*
 *
 */

#ifndef HEMERA_DEVELOPERMODE_TARGET_H
#define HEMERA_DEVELOPERMODE_TARGET_H

#include <QtCore/QObject>
#include <QtCore/QSettings>
#include <QtCore/QSharedPointer>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

#include "hemeradevelopermodeexport.h"
#include "hemeradevelopermodeoperation.h"

class QByteArray;
class QNetworkReply;
class QNetworkRequest;

namespace Hemera {
namespace DeveloperMode {

class Operation;

class Transport;

class Controller;

class Star;
class TargetManager;
class TargetManagerPrivate;

class HemeraDeveloperModeClient_EXPORT NetworkByteArrayOperation : public Operation
{
    Q_OBJECT

public:
    NetworkByteArrayOperation(QNetworkReply *reply);
    ~NetworkByteArrayOperation();
    void startImpl() override;
    QByteArray result() const;
    int status() const;

protected:
    virtual void parseReply(QNetworkReply *reply, int &status, QByteArray &result);

private:
    QByteArray m_result;
    int m_status;
    QNetworkReply *m_reply;
};

class HemeraDeveloperModeClient_EXPORT IsAuthRequiredOperation : public Operation
{
    Q_OBJECT

public:
    IsAuthRequiredOperation(QNetworkReply *reply);
    ~IsAuthRequiredOperation();
    void startImpl() override;
    bool result() const;

private:
    bool m_result;
};


class TargetPrivate;
class HemeraDeveloperModeClient_EXPORT Target : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Target)
    Q_DECLARE_PRIVATE(Target)

    Q_PRIVATE_SLOT(d_func(), void init())

    Q_PROPERTY(bool             online              READ isOnline                 NOTIFY onlineChanged         STORED true)
    Q_PROPERTY(QString          id                  READ id                                                    STORED true)
    Q_PROPERTY(QString          applianceId         READ applianceId                                           STORED true)
    Q_PROPERTY(bool             hasBuildEnvironment READ hasBuildEnvironment                                   STORED true)

    Q_PROPERTY(QString          name                READ name                                                  STORED true)

    Q_PROPERTY(QString          targetName          READ targetName               NOTIFY targetInfoChanged     STORED true)
    Q_PROPERTY(QStringList      installedApps       READ installedApps            NOTIFY targetInfoChanged     STORED true)
    Q_PROPERTY(QStringList      stars               READ stars                    NOTIFY targetInfoChanged     STORED true)

    // Now, for infos on the system
    Q_PROPERTY(QString          hemeraRelease       READ hemeraRelease            NOTIFY targetInfoChanged     STORED true)
    Q_PROPERTY(QStringList      cpuFlags            READ cpuFlags                 NOTIFY targetInfoChanged     STORED true)
    Q_PROPERTY(int              cpuFrequency        READ cpuFrequency             NOTIFY targetInfoChanged     STORED true)
    Q_PROPERTY(int              availableCores      READ availableCores           NOTIFY targetInfoChanged     STORED true)
    Q_PROPERTY(int              totalMemory         READ totalMemory              NOTIFY targetInfoChanged     STORED true)

public:
    typedef QSharedPointer<Target> Ptr;
    typedef QSharedPointer<const Target> ConstPtr;

    virtual ~Target();

    bool isOnline() const;
    QUrl url() const;

    Star *star(const QString &handler);
    QSharedPointer<Controller> developerModeController() const;
    bool hasAcquiredDeveloperModeController() const;

    Operation *updateLocalTargetCache();

    QString id() const;
    QString applianceId() const;
    QString name() const;
    bool hasBuildEnvironment() const;

    QString targetName() const;
    QStringList installedApps() const;
    QStringList stars() const;

    QString hemeraRelease() const;
    QStringList cpuFlags() const;
    int cpuFrequency() const;
    int availableCores() const;
    int totalMemory() const;

    Operation *ensureDeveloperModeController(int timeout = 5000);
    Operation *ensureOnline(int timeout = 5000);

    bool waitForTargetInfo(int timeout = 5000);

    IsAuthRequiredOperation *isAuthRequired(const QString &targetAPIUrl);
    NetworkByteArrayOperation *startAuth(const QString &apiUrl, const QString &capability, const QString &path, const QString &method, int ttl);
    NetworkByteArrayOperation *sendAuthPIN(const QString &apiUrl, QByteArray pairingTempToken, QString pin);

    QString pathToScripts() const;

Q_SIGNALS:
    void onlineChanged(bool online);
    void targetInfoChanged();

    void developerModeControllerChanged();

protected:
    // This tries to auto-discover the transport through its id.
    Target(TargetPrivate &dd, const QString &id, TargetManager *parent);
    // This uses a pre-set transport. If id is not empty, don't detect it.
    Target(TargetPrivate &dd, Transport *transport, TargetManager *parent, const QString &id = QString());
    // This tries to detect the transport from the URL. If id is not empty, don't detect it.
    Target(TargetPrivate &dd, const QUrl &url, TargetManager *parent, const QString &id = QString());
    // Gets the target's settings
    virtual QSettings *settings() = 0;

    void setTypeName(const QString &typeName);

    Ptr sharedFromThis();
    ConstPtr sharedFromThis() const;

    Transport *transport() const;

protected:
    TargetPrivate * const d_ptr;

private Q_SLOTS:
    void setOnline(bool online);
    void appendAuthData(QNetworkRequest &request);

    friend class ApplicationOutput;
    friend class DeployOperation;
    friend class EnsureDeveloperModeControllerOperation;
    friend class EnsureOnlineOperation;
    friend class Star;
    friend class TargetManager;
    friend class TargetManagerPrivate;
};

}
}

#endif // HEMERA_DEVELOPERMODE_TARGET_H
