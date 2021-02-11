/*
 *
 */

#include "hemeradevelopermodetarget_p.h"

#include "hemeradevelopermodecontroller.h"
#include "hemeradevelopermodestar.h"
#include "hemeradevelopermodetargetmanager.h"

#include "transports/hemeradevelopermodetransport.h"

// Our transport(s)
#include "transports/hemeradevelopermodetcptransport.h"
#include "wrapperscripts.h"
#include "hemeradevelopermodetargetmanager_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QPointer>
#include <QtCore/QTimer>

#include <QtNetwork/QHostInfo>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace Hemera {
namespace DeveloperMode {

UpdateLocalCacheOperation::UpdateLocalCacheOperation(Target::Ptr target, QObject *parent)
    : Operation(parent)
    , m_target(target)

{
}

UpdateLocalCacheOperation::~UpdateLocalCacheOperation()
{
}

void UpdateLocalCacheOperation::setThingsToDo(int things)
{
    m_totalThings = things;
    m_leftThings = things;
}

void UpdateLocalCacheOperation::oneThingLessToDo()
{
    --m_leftThings;
    if (m_leftThings <= 0 && !isFinished()) {
        setFinished();
    }
}

void UpdateLocalCacheOperation::startImpl()
{
    setThingsToDo(5);
}

EnsureOnlineOperation::EnsureOnlineOperation(const Target::Ptr &target, int timeout, QObject *parent)
    : Operation(parent)
    , m_target(target)
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setSingleShot(true);
    m_timeoutTimer->setInterval(timeout);
    connect(m_timeoutTimer, &QTimer::timeout, [this] { setFinishedWithError(QStringLiteral("Timeout"), tr("Operation has timed out.")); });
}

EnsureOnlineOperation::~EnsureOnlineOperation()
{
}

void EnsureOnlineOperation::startImpl()
{
    if (m_target->isOnline()) {
        QTimer::singleShot(0, this, SLOT(onOnlineChanged()));
    } else {
        m_target->transport()->startConnecting();
        connect(m_target.data(), &Target::onlineChanged, this, &EnsureOnlineOperation::onOnlineChanged);
        // Timeout timer
        m_timeoutTimer->start();
    }
}

void EnsureOnlineOperation::onOnlineChanged()
{
    if (isFinished()) {
        return;
    }
    m_timeoutTimer->stop();

    if (m_target->isOnline()) {
        setFinished();
    } else {
        setFinishedWithError(QStringLiteral("Target offline"), tr("Target could not be brought online."));
    }
}


EnsureDeveloperModeControllerOperation::EnsureDeveloperModeControllerOperation(const Target::Ptr &target, int timeout, QObject *parent)
    : Operation(parent)
    , m_target(target)
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setSingleShot(true);
    m_timeoutTimer->setInterval(timeout);
    connect(m_timeoutTimer, &QTimer::timeout, [this] { setFinishedWithError(QStringLiteral("Timeout"), tr("Operation has timed out.")); });
}

EnsureDeveloperModeControllerOperation::~EnsureDeveloperModeControllerOperation()
{
}

void EnsureDeveloperModeControllerOperation::startImpl()
{
    connect(m_target->ensureOnline(m_timeoutTimer->interval()), &Operation::finished, [this] (Operation *op) {
        if (op->isError()) {
            setFinishedWithError(op->errorName(), op->errorMessage());
            return;
        }

        if (m_target->developerModeController()) {
            QTimer::singleShot(0, this, SLOT(onDeveloperModeControllerChanged()));
        } else {
            m_target->d_func()->requestDeveloperModeDevice();
            connect(m_target.data(), &Target::developerModeControllerChanged, this, &EnsureDeveloperModeControllerOperation::onDeveloperModeControllerChanged);
            // Timeout timer
            m_timeoutTimer->start();
        }
    });
}

void EnsureDeveloperModeControllerOperation::onDeveloperModeControllerChanged()
{
    if (isFinished()) {
        return;
    }
    m_timeoutTimer->stop();

    if (m_target->developerModeController()) {
        setFinished();
    } else {
        setFinishedWithError(QStringLiteral("Developer Mode Controller offline"), tr("Could not retrieve developer mode controller for target."));
    }
}


void TargetPrivate::requestDeveloperModeDevice()
{
    if (!transport) {
        return;
    }

    developerModeController.clear();

    HyperspaceStream *stream = transport->streamRequest(QStringLiteral("com.ispirata.Hemera.DeveloperMode/control"));
    QObject::connect(stream, &HyperspaceStream::stateChanged, [this, stream] (QAbstractSocket::SocketState state) {
        Q_Q(Target);
        if (state == QAbstractSocket::ConnectedState) {
            developerModeController = Controller::Ptr(new Controller(q->sharedFromThis(), stream));

            Q_EMIT q->developerModeControllerChanged();
        } else if (state == QAbstractSocket::ClosingState || state == QAbstractSocket::UnconnectedState) {
            developerModeController.clear();
            Q_EMIT q->developerModeControllerChanged();
        }
    });

    // TODO: Error management
}

void TargetPrivate::cacheDeviceInfo(const QByteArray &infoData)
{
    Q_Q(Target);
    QSettings *settings = q->settings();
    settings->setValue(QStringLiteral("cachedDeviceInfo"), infoData);
    settings->sync();
    delete settings;
}

QByteArray TargetPrivate::cachedDeviceInfo()
{
    Q_Q(Target);
    QSettings *settings = q->settings();
    QByteArray info = settings->value(QStringLiteral("cachedDeviceInfo")).toByteArray();
    delete settings;
    return info;
}

void TargetPrivate::restoreCachedDeviceInfo()
{
    QByteArray info = cachedDeviceInfo();
    if (Q_LIKELY(!info.isEmpty())) {
        setDeviceInfo(info, true);
    }
}

void TargetPrivate::setDeviceInfo(const QByteArray &infoData, bool cached)
{
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(infoData, &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "Parse error 1!";
        return;
    }

    if (!document.isObject()) {
        qDebug() << "Parse error 2!";
        return;
    }

    QJsonObject jsonObject = document.object();

    auto variantListToStringList = [this] (const QVariantList &list) -> QStringList { QStringList ret; ret.reserve(list.size());
                                                                                        for (const QVariant &v : list) ret.append(v.toString()); return ret; };

    // Ok, now hold on for a minute. If we were already online, it's likely nothing has changed at all. Verify.
    if (installedApps == variantListToStringList(jsonObject.value(QStringLiteral("installedApps")).toArray().toVariantList()) &&
        stars == variantListToStringList(jsonObject.value(QStringLiteral("stars")).toArray().toVariantList())) {
        // Nothing really changed.
        return;
    }

    // Set all properties
    applianceName = jsonObject.value(QStringLiteral("applianceName")).toString();
    targetName = jsonObject.value(QStringLiteral("targetName")).toString();
    if (id.isEmpty()) {
        id = jsonObject.value(QStringLiteral("hardwareId")).toString();
    }
    applianceId = jsonObject.value(QStringLiteral("applianceId")).toString();
    installedApps = variantListToStringList(jsonObject.value(QStringLiteral("installedApps")).toArray().toVariantList());
    isProductionBoard = jsonObject.value(QStringLiteral("isProductionBoard")).toBool();
    hasBuildEnvironment = jsonObject.value(QStringLiteral("hasBuildEnvironment")).toBool();
    stars = variantListToStringList(jsonObject.value(QStringLiteral("stars")).toArray().toVariantList());

    architecture = jsonObject.value(QStringLiteral("architecture")).toString();
    hemeraRelease = jsonObject.value(QStringLiteral("hemeraRelease")).toString();
    cpuFlags = variantListToStringList(jsonObject.value(QStringLiteral("cpuFlags")).toArray().toVariantList());
    cpuFrequency = jsonObject.value(QStringLiteral("cpuFrequency")).toInt();
    availableCores = jsonObject.value(QStringLiteral("availableCores")).toInt();
    totalMemory = jsonObject.value(QStringLiteral("totalMemory")).toInt();

    if (hasBuildEnvironment) {
        // We need to fetch build environment info as well.
        if (!cached) {
            // Pull
            QUrl deviceInfo = transport->urlForRelativeTarget(QStringLiteral("/com.ispirata.Hemera.DeviceInfo/system/build-environment"));

            QNetworkReply *reply = nam->get(QNetworkRequest(deviceInfo));
            QObject::connect(reply, &QNetworkReply::finished, [this, reply] {
                if (reply->error() != QNetworkReply::NoError) {
                    qWarning() << "Target::refreshInfoBuildEnvironment ERROR" << reply->errorString();
                } else {
                    QByteArray info = reply->readAll();
                    if (info != cachedBuildEnvironmentInfo()) {
                        setBuildEnvironmentInfo(info, false);
                        // Cache dat as well
                        cacheBuildEnvironmentInfo(info);
                    }
                }

                reply->deleteLater();
            });
        } else {
            restoreCachedBuildEnvironmentInfo();
        }
    }

    Q_Q(Target);

    // Notify.
    Q_EMIT q->targetInfoChanged();
}

void TargetPrivate::cacheBuildEnvironmentInfo(const QByteArray &infoData)
{
    Q_Q(Target);
    QSettings *settings = q->settings();
    settings->setValue(QStringLiteral("cachedBuildEnvironmentInfo"), infoData);
    settings->sync();
    delete settings;
}

QByteArray TargetPrivate::cachedBuildEnvironmentInfo()
{
    Q_Q(Target);
    QSettings *settings = q->settings();
    QByteArray info = settings->value(QStringLiteral("cachedBuildEnvironmentInfo")).toByteArray();
    delete settings;
    return info;
}

void TargetPrivate::restoreCachedBuildEnvironmentInfo()
{
    QByteArray info = cachedBuildEnvironmentInfo();
    if (Q_LIKELY(!info.isEmpty())) {
        setBuildEnvironmentInfo(info, true);
    }
}

void TargetPrivate::setBuildEnvironmentInfo(const QByteArray &infoData, bool cached)
{
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(infoData, &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "Parse error 1!";
        return;
    }

    if (!document.isObject()) {
        qDebug() << "Parse error 2!";
        return;
    }

    QJsonObject jsonObject = document.object();

    auto variantListToStringList = [this] (const QVariantList &list) -> QStringList { QStringList ret; ret.reserve(list.size());
                                                                                        for (const QVariant &v : list) ret.append(v.toString()); return ret; };

    // Ok, now hold on for a minute. If we were already online, it's likely nothing has changed at all. Verify.
    if (buildArchitectures == variantListToStringList(jsonObject.value(QStringLiteral("architectures")).toArray().toVariantList())) {
        // Nothing really changed.
        return;
    }

    buildArchitectures = variantListToStringList(jsonObject.value(QStringLiteral("architectures")).toArray().toVariantList());
    defaultBuildArchitecture = jsonObject.value(QStringLiteral("defaultArchitecture")).toBool();

    Q_Q(Target);

    // Notify.
    Q_EMIT q->targetInfoChanged();
}

void TargetPrivate::init()
{
    // Check scripts
    if (!WrapperScripts::checkScripts(typeName, name)) {
        qDebug() << "Creating scripts for target" << name;
        // We will try our best.
        QString hsdk;
#if defined(Q_OS_WIN)
        hsdk = QStringLiteral("hsdk.exe");
#else
        hsdk = QStringLiteral("hsdk");
#endif
        if (!WrapperScripts::createScripts(typeName, name, QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + QDir::separator() + hsdk))) {
            qWarning() << "Could not create scripts for target" << name;
        }
    }

    // Restore the cache, if any.
    restoreCachedDeviceInfo();
}


// Only TCP for now...
Target::Target(TargetPrivate& dd, const QString& id, TargetManager* parent)
    : Target(dd, new TCPTransport(id), parent, QString())
{
}

Target::Target(TargetPrivate& dd, Transport* transport, TargetManager* parent, const QString& id)
    : QObject(parent)
    , d_ptr(&dd)
{
    Q_D(Target);

    if (!id.isEmpty()) {
        d->id = id;
    }

    d->transport = transport;
    if (transport->isOnline()) {
        setOnline(true);
    }

    connect(transport, &Transport::onlineChanged, this, &Target::setOnline);
    QTimer::singleShot(0, this, SLOT(init()));
}

Target::Target(TargetPrivate& dd, const QUrl& url, TargetManager* parent, const QString &id)
    : QObject(parent)
    , d_ptr(&dd)
{
    Q_D(Target);

    if (!id.isEmpty()) {
        d->id = id;
    }

    // Only TCP for now.
    if (url.scheme() == QStringLiteral("https") || url.scheme() == QStringLiteral("http")) {
        d->transport = new TCPTransport(url, this);
    } else {
        qWarning() << "No transport available for url" << url;
        return;
    }

    if (d->transport->isOnline()) {
        setOnline(true);
    }

    connect(d->transport, &Transport::onlineChanged, this, &Target::setOnline);
    QTimer::singleShot(0, this, SLOT(init()));
}

Target::~Target()
{
    Q_D(Target);

    if (d->transport) {
        d->transport->deleteLater();
    }

    delete d_ptr;
}

QUrl Target::url() const
{
    if (transport()) {
        return transport()->urlForRelativeTarget(QStringLiteral("/"));
    }

    return QUrl();
}

void Target::setOnline(bool online)
{
    Q_D(Target);

    if (online == d->isOnline) {
        // No.
        return;
    }

    // Notify
    d->isOnline = online;
    Q_EMIT onlineChanged(online);

    if (online) {
        // Cool. Gather infos.
        QUrl deviceInfo = d->transport->urlForRelativeTarget(QStringLiteral("/com.ispirata.Hemera.DeviceInfo/system/info"));

        QNetworkReply *reply = d->nam->get(QNetworkRequest(deviceInfo));
        QObject::connect(reply, &QNetworkReply::finished, [this, d, reply] {
            if (reply->error() != QNetworkReply::NoError) {
                qWarning() << "Target::refreshInfo ERROR" << reply->errorString();
            } else {
                QByteArray info = reply->readAll();
                if (info != d->cachedDeviceInfo()) {
                    d->setDeviceInfo(info, false);
                    // Cache dat as well
                    d->cacheDeviceInfo(info);
                }
            }

            reply->deleteLater();
        });
    } else {
        // We do not clear. The cache needs to stay alive.
    }
}

bool Target::waitForTargetInfo(int timeout)
{
    Q_D(Target);

    if (!d->applianceName.isEmpty()) {
        return true;
    }

    QTimer timer(this);
    timer.setInterval(timeout);
    timer.start();

    // We need to ensure the target is online.
    ensureOnline(timeout);

    QEventLoop e;

    connect(&timer, &QTimer::timeout, &e, &QEventLoop::quit);
    connect(this, &Target::targetInfoChanged, &e, &QEventLoop::quit);

    e.exec();

    return !d->applianceName.isEmpty();
}

void Target::setTypeName(const QString& typeName)
{
    Q_D(Target);
    d->typeName = typeName;
}

bool Target::hasAcquiredDeveloperModeController() const
{
    Q_D(const Target);
    return !d->developerModeController.isNull();
}

Controller::Ptr Target::developerModeController() const
{
    Q_D(const Target);
    return d->developerModeController;
}

Operation* Target::updateLocalTargetCache()
{
    return new UpdateLocalCacheOperation(sharedFromThis(), this);
}

Star* Target::star(const QString &s)
{
    Q_D(Target);

    if (!d->stars.contains(s)) {
        return Q_NULLPTR;
    }
    if (!d->starObjects.contains(s)) {
        d->starObjects.insert(s, new Star(sharedFromThis(), s));
        // Add this as a context, to prevent from calling this lambda when this is destroyed.
        connect(d->starObjects.value(s), &QObject::destroyed, this, [d, s] { d->starObjects.remove(s); });
    }

    return d->starObjects.value(s);
}

QString Target::name() const
{
    Q_D(const Target);
    return d->name;
}

Transport *Target::transport() const
{
    Q_D(const Target);
    return d->transport;
}

int Target::availableCores() const
{
    Q_D(const Target);
    return d->availableCores;
}

QString Target::id() const
{
    Q_D(const Target);
    return d->id;
}

QString Target::applianceId() const
{
    Q_D(const Target);
    return d->applianceId;
}

bool Target::hasBuildEnvironment() const
{
    Q_D(const Target);
    return d->hasBuildEnvironment;
}

QString Target::targetName() const
{
    Q_D(const Target);
    return d->targetName;
}

QStringList Target::cpuFlags() const
{
    Q_D(const Target);
    return d->cpuFlags;
}

int Target::cpuFrequency() const
{
    Q_D(const Target);
    return d->cpuFrequency;
}

QString Target::hemeraRelease() const
{
    Q_D(const Target);
    return d->hemeraRelease;
}

QStringList Target::installedApps() const
{
    Q_D(const Target);
    return d->installedApps;
}

bool Target::isOnline() const
{
    Q_D(const Target);
    return d->isOnline;
}

QStringList Target::stars() const
{
    Q_D(const Target);
    return d->stars;
}

int Target::totalMemory() const
{
    Q_D(const Target);
    return d->totalMemory;
}

IsAuthRequiredOperation *Target::isAuthRequired(const QString &targetAPIUrl)
{
    Q_D(Target);
    QUrl testUrl = d->transport->urlForRelativeTarget(targetAPIUrl);
    QNetworkRequest getRequest(testUrl);

    return new IsAuthRequiredOperation(d->nam->get(getRequest));
}

NetworkByteArrayOperation *Target::startAuth(const QString &apiUrl, const QString &capability, const QString &path, const QString &method, int ttl)
{
    Q_D(Target);
    QUrl pairUrl = d->transport->urlForRelativeTarget(QStringLiteral("%1/auth/pair").arg(apiUrl));

    QJsonObject request;
    request.insert(QStringLiteral("ttl"), QJsonValue(ttl));
    request.insert(QStringLiteral("capability"), QJsonValue(capability));
    request.insert(QStringLiteral("path"), QJsonValue(path));
    request.insert(QStringLiteral("method"), QJsonValue(method));

    QJsonArray requestsArray;
    requestsArray.append(QJsonValue(request));

    QJsonObject obj;
    obj.insert(QStringLiteral("permissionsStatements"), requestsArray);

    QJsonDocument doc;
    doc.setObject(obj);
    QByteArray data = doc.toJson();

    QNetworkRequest postRequest(pairUrl);
    postRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    return new NetworkByteArrayOperation(d->nam->post(postRequest, data));
}

NetworkByteArrayOperation *Target::sendAuthPIN(const QString &apiUrl, QByteArray pairingTempToken, QString pin)
{
    Q_D(Target);

    QUrl pinUrl = d->transport->urlForRelativeTarget(QStringLiteral("%1/auth/pin").arg(apiUrl));

    QJsonObject obj;
    obj.insert(QStringLiteral("pin"), QJsonValue(pin));
    obj.insert(QStringLiteral("requestsId"), QJsonValue(QString::fromLatin1(pairingTempToken)));

    QJsonDocument doc;
    doc.setObject(obj);
    QByteArray data = doc.toJson();

    QNetworkRequest postRequest(pinUrl);
    postRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    NetworkByteArrayOperation *operation = new NetworkByteArrayOperation(d->nam->post(postRequest, data));
    QObject::connect(operation, &Operation::finished, [this, operation] {
        Q_D(Target);

        d->authToken = operation->result();
        qDebug() << "Pairing OK. Auth token: " << d->authToken;

        QSettings *target = settings();
        target->setValue("authToken", d->authToken);
        target->sync();
        delete target;
    });

    return operation;
}

void Target::appendAuthData(QNetworkRequest &request)
{
    Q_D(Target);
    if (!d->authToken.isEmpty()) {
        request.setRawHeader("Authorization", d->authToken);
    }
}

QString Target::pathToScripts() const
{
    Q_D(const Target);
    return WrapperScripts::targetConfigurationPath(d->typeName, name());
}

Target::ConstPtr Target::sharedFromThis() const
{
    return TargetManager::instance()->d->fromRawPointer(this);
}

Target::Ptr Target::sharedFromThis()
{
    return TargetManager::instance()->d->fromRawPointer(this);
}

Operation* Target::ensureDeveloperModeController(int timeout)
{
    return new EnsureDeveloperModeControllerOperation(sharedFromThis(), timeout, this);
}

Operation* Target::ensureOnline(int timeout)
{
    return new EnsureOnlineOperation(sharedFromThis(), timeout, this);
}


NetworkByteArrayOperation::NetworkByteArrayOperation(QNetworkReply *reply)
    : m_status(-1), m_reply(reply)
{
    QObject::connect(reply, &QNetworkReply::finished, [this, reply] {
        parseReply(reply, m_status, m_result);
    });
}

NetworkByteArrayOperation::~NetworkByteArrayOperation()
{
}


void NetworkByteArrayOperation::startImpl()
{
}

QByteArray NetworkByteArrayOperation::result() const
{
    return m_result;
}

int NetworkByteArrayOperation::status() const
{
    return m_status;
}

void NetworkByteArrayOperation::parseReply(QNetworkReply *reply, int &status, QByteArray &result)
{
    if (reply->error() != QNetworkReply::NoError) {
        status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        result = reply->errorString().toLatin1();
    } else {
        status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        result = reply->readAll();
    }
    setFinished();
}

IsAuthRequiredOperation::IsAuthRequiredOperation(QNetworkReply *reply)
    : m_result(false)
{
    QObject::connect(reply, &QNetworkReply::finished, [this, reply] {
        // 401 = Unauthroized
        m_result = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401;
        setFinished();
    });
}

IsAuthRequiredOperation::~IsAuthRequiredOperation()
{
}

void IsAuthRequiredOperation::startImpl()
{
}

bool IsAuthRequiredOperation::result() const
{
    return m_result;
}

}
}

// For Q_PRIVATE_SLOT
#include "moc_hemeradevelopermodetarget.cpp"
