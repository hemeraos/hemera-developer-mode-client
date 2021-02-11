#include "hemeradevelopermodehyperspacestream.h"

namespace Hemera {
namespace DeveloperMode {

class HyperspaceStream::Private
{
public:
    Private() : state(QAbstractSocket::UnconnectedState) {}

    QAbstractSocket::SocketState state;
};

HyperspaceStream::HyperspaceStream(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
}

HyperspaceStream::~HyperspaceStream()
{
    delete d;
}

void HyperspaceStream::setState(QAbstractSocket::SocketState state)
{
    if (Q_UNLIKELY(state == d->state)) {
        return;
    }

    d->state = state;
    Q_EMIT stateChanged(state);
}

QAbstractSocket::SocketState HyperspaceStream::state() const
{
    return d->state;
}

}
}
