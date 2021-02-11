#ifndef HEMERA_DEVELOPERMODE_TRANSPORT_H
#define HEMERA_DEVELOPERMODE_TRANSPORT_H

#include <hemeradevelopermodeoperation.h>

#include <QtCore/QHash>

class QIODevice;

namespace Hemera {
namespace DeveloperMode {

class HyperspaceStream;

class Transport : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Transport)

public:
    virtual ~Transport();

    bool isOnline() const;

//     static QString transportTypeToString(Target::HyperspaceTransportType transport);
//     static HyperspaceTransportType transportStringToType(const QString &transport);

    /// Those NEED a trailing slash!
    virtual QUrl urlForRelativeTarget(const QString &relativeTarget) = 0;
    virtual HyperspaceStream *streamRequest(const QString &relativeTarget,
                                            const QHash< QByteArray, QByteArray > &arguments = QHash< QByteArray, QByteArray >(), QObject *parent = Q_NULLPTR) = 0;

    virtual void startConnecting() = 0;

protected Q_SLOTS:
    virtual void checkHostIsAlive() = 0;

Q_SIGNALS:
    void onlineChanged(bool online);

protected:
    explicit Transport(QObject *parent = Q_NULLPTR);

    void setOnline(bool online);
    /// If this is true, then the transport will check if the host is alive every now and then to
    /// ensure it is alive.
    void setNeedsKeepAliveCheck(bool keepAliveCheck);

private:
    class Private;
    Private * const d;
};
}
}

#endif // HEMERA_DEVELOPERMODE_TRANSPORT_H
