#include "hemeradevelopermodeglobalobjects_p.h"

#include <QtCore/QCoreApplication>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

namespace Hemera {
namespace DeveloperMode {

class GlobalObjects::Private
{
public:
    QNetworkAccessManager *networkAccessManager;
};

class GlobalObjectsHelper
{
public:
    GlobalObjectsHelper() : q(0) {}
    ~GlobalObjectsHelper() {
        delete q;
    }
    GlobalObjects *q;
};

Q_GLOBAL_STATIC(GlobalObjectsHelper, s_globalGlobalObjects)

GlobalObjects * const GlobalObjects::instance()
{
    if (!s_globalGlobalObjects()->q) {
        new GlobalObjects();
    }

    return s_globalGlobalObjects()->q;
}

static void cleanup_nam()
{
    if (s_globalGlobalObjects->q) {
        delete GlobalObjects::instance()->networkAccessManager();
        s_globalGlobalObjects->q->deleteLater();
    }
}

GlobalObjects::GlobalObjects()
    : QObject()
    , d(new Private)
{
    Q_ASSERT(!s_globalGlobalObjects->q);
    s_globalGlobalObjects->q = this;

    // Create objects
    d->networkAccessManager = new QNetworkAccessManager(this);

    connect(d->networkAccessManager, &QNetworkAccessManager::sslErrors, [this] (QNetworkReply * reply, const QList<QSslError> &errors) {
        reply->ignoreSslErrors();
    });

    // Add a post routine for correct deletion of QNAM.
    qAddPostRoutine(cleanup_nam);
}

GlobalObjects::~GlobalObjects()
{
    delete d;
    s_globalGlobalObjects()->q = Q_NULLPTR;
}

QNetworkAccessManager* GlobalObjects::networkAccessManager()
{
    return d->networkAccessManager;
}

}
}
