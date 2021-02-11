#ifndef _HYPERDISCOVERY_H_
#define _HYPERDISCOVERY_H_

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QElapsedTimer>
#include <QtCore/QStringList>
#include <QtCore/QSet>

#include <QtNetwork/QUdpSocket>

#include "hemeradevelopermodeexport.h"

class Service;
class DiscoveryCore;

namespace Hemera {
namespace DeveloperMode {

class HemeraDeveloperModeClient_EXPORT HyperDiscoveryClient : public QObject
{
    Q_OBJECT

public:
    explicit HyperDiscoveryClient(QObject *parent = 0);
    virtual ~HyperDiscoveryClient();
    void scanCapabilities(const QList<QByteArray> &capabilities);
    void appendAnswer(QByteArray &a, const QByteArray &cap, quint16 ttl, quint16 port);

Q_SIGNALS:
    int capabilityDiscovered(QByteArray capability, const QHostAddress &address, int port, int ttl);

public slots:
    void packetReceived();

private:
    QUdpSocket *udpSocket;
    void buildAnswer(QByteArray &answer, const QByteArray &capability);

    QMultiHash<QByteArray, Service *> services;
    QSet<QByteArray> m_capabilities;
};

}
}

#endif

