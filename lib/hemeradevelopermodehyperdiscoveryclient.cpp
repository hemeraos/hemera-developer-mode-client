#include "hemeradevelopermodehyperdiscoveryclient.h"

#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtNetwork/QNetworkAddressEntry>
#include <QtNetwork/QNetworkInterface>

#define HYPERDISCOVERY_SCAN_PACKETTYPE 's'
#define HYPERDISCOVERY_ANSWER_PACKETTYPE 'a'

#define HYPERDISCOVERY_PORT 44389

namespace Hemera {
namespace DeveloperMode {

HyperDiscoveryClient::HyperDiscoveryClient(QObject *parent)
    : QObject(parent)
{
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::Any, HYPERDISCOVERY_PORT);
    connect(udpSocket, &QUdpSocket::readyRead, this, &HyperDiscoveryClient::packetReceived);
}

HyperDiscoveryClient::~HyperDiscoveryClient()
{
}

void HyperDiscoveryClient::scanCapabilities(const QList<QByteArray> &capabilities)
{
    /*
        Question packet structure: |'s' (quint8)|CAPABILITIES|0 (quint8)|
    */
    QByteArray discoveryPacket;
    discoveryPacket.append(HYPERDISCOVERY_SCAN_PACKETTYPE);

    for (const QByteArray &q : capabilities) {
        /*
            Capability: | capability length (quint8) | capability string |
        */
        quint8 len = q.length();
        discoveryPacket.append(len);
        discoveryPacket.append(q);
    }
    //End of capabilities list
    discoveryPacket.append((char) 0x00);

    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    for (int i = 0; i < ifaces.count(); i++){
        QList<QNetworkAddressEntry> addrs = ifaces[i].addressEntries();
        for (int j = 0; j < addrs.size(); j++) {
            if ((addrs[j].ip().protocol() == QAbstractSocket::IPv4Protocol) && (addrs[j].broadcast().toString() != "")) {
                udpSocket->writeDatagram(discoveryPacket, addrs[j].broadcast(), HYPERDISCOVERY_PORT);
            }
        }
    }
}

void HyperDiscoveryClient::appendAnswer(QByteArray &a, const QByteArray &cap, quint16 ttl, quint16 port)
{
    /*
     Answer structure: |Capability length (quint8)|Extra data length (quint8)|TTL (quint16)|Port (quint16)|
                       |Capability|Extra data|
    */

    //capability.length() and extra data field length
    a.append((char) cap.length());
    a.append((char) 0x00); //not yet used

    //TTL
    a.append((char) (ttl & 0xFF));
    a.append((char) ((ttl >> 8) & 0xFF));

    //Port
    a.append((char) (port & 0xFF));
    a.append((char) ((port >> 8) & 0xFF));

    a.append(cap);
    //extra data field is not yet used but we should append it here
}

void HyperDiscoveryClient::buildAnswer(QByteArray &answer, const QByteArray &capability)
{
    /*
        Answer packet: |'a' (quint8)| ANSWERS |
    */
    if (answer.size() == 0){
        answer.append(HYPERDISCOVERY_ANSWER_PACKETTYPE);
    }

    if (capability.endsWith('*')){
        QByteArray searchPrefix = capability.left(capability.length() - 1);

        for (QSet<QByteArray>::const_iterator it = m_capabilities.constBegin(); it != m_capabilities.constEnd(); ++it) {
            if ((*it) == searchPrefix){
                appendAnswer(answer, (*it), 3600, 80); //FIXME
            }
        }

    }else if (m_capabilities.contains(capability)) {
        appendAnswer(answer, capability, 3600, 80);
    }else{
        return;
    }
}

void HyperDiscoveryClient::packetReceived()
{
    QList<QByteArray> capabilitiesAnnounced;
    QList<QByteArray> capabilitiesPurged;

    while (udpSocket->hasPendingDatagrams()){
        int pendingSize = udpSocket->pendingDatagramSize();
        QByteArray data;
        data.resize(pendingSize);
        QHostAddress client;
        quint16 clientPort;
        udpSocket->readDatagram(data.data(), pendingSize, &client, &clientPort);

        int rPos = 0;
        if (data.at(rPos) == HYPERDISCOVERY_ANSWER_PACKETTYPE) {
            rPos++;

            Q_FOREVER {
                if (rPos >= data.length()) break; //CHECK OUT OF BOUNDS
                quint8 capLen = data.at(rPos);
                if (capLen == 0){
                    //NULL length capability found, we don't have more data
                    break;
                }
                rPos++;

                if (rPos >= data.length()) break; //CHECK OUT OF BOUNDS
                quint8 txtLen = data.at(rPos);
                rPos++;

                if (rPos + 1 >= data.length()) break; //CHECK OUT OF BOUNDS
                quint16 ttl = data.at(rPos) | data.at(rPos + 1) << 8;
                rPos += 2;

                if (rPos + 1>= data.length()) break; //CHECK OUT OF BOUNDS
                quint16 port = ((quint8) data.at(rPos)) | (((quint8) data.at(rPos + 1)) << 8);
                rPos += 2;

                if (rPos + capLen > data.length()) break; //CHECK OUT OF BOUNDS
                QByteArray cap(data.data() + rPos, capLen);
                rPos += capLen;

                if (rPos + txtLen> data.length()) break; //CHECK OUT OF BOUNDS
                QByteArray txt(data.data() + rPos, txtLen);
                rPos += txtLen;

                emit capabilityDiscovered(cap, client, port, ttl);
            }
        }
    }
}

}
}
