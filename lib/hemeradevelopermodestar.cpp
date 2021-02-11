#include "hemeradevelopermodestar.h"

#include "hemeradevelopermodecontroller.h"
#include "hemeradevelopermodetarget.h"
#include "hemeradevelopermodeglobalobjects_p.h"

#include "transports/hemeradevelopermodetransport.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtCore/QUrl>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#define HEMERA_DEVICE_INFO_URL "/com.ispirata.Hemera.DeviceInfo/stars/"

#define CRAFT_DEVINFO_REQUEST(HANDLER) \
QNetworkRequest request(target->transport()->urlForRelativeTarget(HEMERA_DEVICE_INFO_URL + HANDLER));\
request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");\
QVariantMap data

namespace Hemera {
namespace DeveloperMode {

class Star::Private {
public:
    Private(Star *q) : q(q), valid(false), nam(GlobalObjects::instance()->networkAccessManager())
                             , display(DisplayType::Unknown), phase(Star::Phase::Unknown) {}

    QJsonObject parseReply(const QByteArray &data, bool *error = Q_NULLPTR);
    void refreshInfo();

    void onOnlineChanged(bool online);

    Star * const q;

    bool valid;

    Target::Ptr target;
    QNetworkAccessManager *nam;
    QString star;

    DisplayType display;
    QString activeOrbit;
    QString residentOrbit;
    Phase phase;

    bool inhibitionActive;
    QVariantMap inhibitionReasons;
    QString properties;
};

QJsonObject Star::Private::parseReply(const QByteArray& data, bool* errorValue)
{
    QJsonParseError error;
    if (errorValue != Q_NULLPTR) {
        *errorValue = false;
    }

    QJsonDocument document = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        if (errorValue != Q_NULLPTR) {
            *errorValue = true;
        }
        return QJsonObject();
    }

    if (!document.isObject()) {
        if (errorValue != Q_NULLPTR) {
            *errorValue = true;
        }
        return QJsonObject();
    }

    return document.object();
}

void Star::Private::refreshInfo()
{
    // Floooood!!
    {
        CRAFT_DEVINFO_REQUEST(star);
        QNetworkReply *reply = nam->get(request);
        QObject::connect(reply, &QNetworkReply::finished, [this, reply] {
            if (reply->error() == QNetworkReply::NoError) {
                QJsonObject obj = parseReply(reply->readAll());
                if (phase != static_cast< Phase >(obj.value(QStringLiteral("phase")).toInt())) {
                    phase = static_cast< Phase >(obj.value(QStringLiteral("phase")).toInt());
                    Q_EMIT q->phaseChanged(phase);
                }
                if (activeOrbit != obj.value(QStringLiteral("activeOrbit")).toString()) {
                    activeOrbit = obj.value(QStringLiteral("activeOrbit")).toString();
                    Q_EMIT q->activeOrbitChanged(activeOrbit);
                }
                if (inhibitionActive != obj.value(QStringLiteral("inhibitionActive")).toBool() ||
                    inhibitionReasons != obj.value(QStringLiteral("inhibitionReasons")).toObject().toVariantMap()) {
                    inhibitionActive = obj.value(QStringLiteral("inhibitionActive")).toBool();
                    inhibitionReasons = obj.value(QStringLiteral("inhibitionReasons")).toObject().toVariantMap();
                    Q_EMIT q->inhibitionChanged();
                }

                // These go unchanged, so let's just add them.
                // TODO: Gotta implement this in Gravity
                display = DisplayType::X11;// static_cast< DisplayType >(obj.value(QStringLiteral("displayType")).toInt());
                residentOrbit = obj.value(QStringLiteral("residentOrbit")).toString();
                properties = obj.value(QStringLiteral("properties")).toString();

                // Check validity now
                bool newValidity = target->isOnline() && display != DisplayType::Unknown && target->stars().contains(star);
                if (newValidity != valid) {
                    valid = newValidity;
                    Q_EMIT q->validityChanged(valid);
                }
            }
        });
    }
}

void Star::Private::onOnlineChanged(bool online)
{
    if (valid == online) {
        return;
    }

    if (online) {
        // Retrieve info
        refreshInfo();
    } else {
        valid = false;
        Q_EMIT q->validityChanged(valid);
    }
}


Star::Star(const Target::Ptr &parent, const QString& star)
    : QObject(parent.data())
    , d(new Private(this))
{
    d->target = parent;
    d->star = star;

    if (!d->target->stars().contains(star)) {
        // wat
        return;
    }

    // Monitor target proactively
    connect(d->target.data(), SIGNAL(onlineChanged(bool)), this, SLOT(onOnlineChanged(bool)));
    d->onOnlineChanged(d->target->isOnline());
}

Star::~Star()
{
    delete d;
}

bool Star::waitForValid(int timeout)
{
    if (d->valid) {
        return true;
    }

    QTimer timer(this);
    timer.setInterval(timeout);
    timer.start();

    QEventLoop e;

    connect(&timer, &QTimer::timeout, &e, &QEventLoop::quit);
    connect(this, &Star::validityChanged, &e, &QEventLoop::quit);

    e.exec();

    return d->valid;
}

QString Star::activeOrbit() const
{
    return d->activeOrbit;
}

Star::DisplayType Star::display() const
{
    return d->display;
}

QVariantMap Star::inhibitionReasons() const
{
    return d->inhibitionReasons;
}

bool Star::isInhibitionActive() const
{
    return d->inhibitionActive;
}

bool Star::isValid()
{
    return d->valid;
}

QString Star::name() const
{
    return d->star;
}

QString Star::properties() const
{
    return d->properties;
}

QString Star::residentOrbit() const
{
    return d->residentOrbit;
}

Star::Phase Star::phase() const
{
    return d->phase;
}

Target::Ptr Star::target()
{
    return d->target;
}

}
}

// For Q_PRIVATE_SLOT
#include "moc_hemeradevelopermodestar.cpp"
