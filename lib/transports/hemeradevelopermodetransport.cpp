#include "hemeradevelopermodetransport.h"

#include <QtCore/QTimer>

namespace Hemera {
namespace DeveloperMode {

class Transport::Private
{
public:
    Private(Transport *q) : online(false), keepAliveTimer(new QTimer(q))
    {
        keepAliveTimer->setInterval(10000);
        keepAliveTimer->setSingleShot(false);
        connect(keepAliveTimer, &QTimer::timeout, q, &Transport::checkHostIsAlive);
    }

    bool online;
    QTimer *keepAliveTimer;
};

Transport::Transport(QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
}

Transport::~Transport()
{
    delete d;
}

bool Transport::isOnline() const
{
    return d->online;
}

void Transport::setOnline(bool online)
{
    if (d->online != online) {
        d->online = online;
        Q_EMIT onlineChanged(online);
    }
}

void Transport::setNeedsKeepAliveCheck(bool keepAliveCheck)
{
    if (keepAliveCheck) {
        d->keepAliveTimer->start();
    } else {
        d->keepAliveTimer->stop();
    }
}

}
}
