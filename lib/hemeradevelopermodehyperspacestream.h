#ifndef HEMERA_DEVELOPERMODE_HYPERSPACESTREAM_H
#define HEMERA_DEVELOPERMODE_HYPERSPACESTREAM_H

#include <QtCore/QObject>

#include <QtNetwork/QAbstractSocket>

namespace Hemera {
namespace DeveloperMode {

class HyperspaceStream : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(HyperspaceStream)

public:
    virtual ~HyperspaceStream();

    virtual void close() = 0;

    virtual QIODevice *device() = 0;

    QAbstractSocket::SocketState state() const;

Q_SIGNALS:
    void stateChanged(QAbstractSocket::SocketState state);
    void error(QAbstractSocket::SocketError error);

protected:
    explicit HyperspaceStream(QObject *parent = Q_NULLPTR);

    void setState(QAbstractSocket::SocketState state);

private:
    class Private;
    Private * const d;
};
}
}

#endif // HEMERA_DEVELOPERMODE_HYPERSPACESTREAM_H
