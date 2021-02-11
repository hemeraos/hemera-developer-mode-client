#ifndef HEMERA_DEVELOPERMODE_TCPTRANSPORT_H
#define HEMERA_DEVELOPERMODE_TCPTRANSPORT_H

#include "hemeradevelopermodetransport.h"
#include <hemeradevelopermodehyperspacestream.h>

#include <QtCore/QUrl>
#include <QtCore/QPointer>
#include <QtNetwork/QTcpSocket>

class QNetworkReply;
class QTimer;

namespace Hemera {
namespace DeveloperMode {

class HyperDiscoveryClient;

class QReplyTimeout : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QReplyTimeout)

public:
    explicit QReplyTimeout(QNetworkReply *reply, const int timeout);
    virtual ~QReplyTimeout();

private Q_SLOTS:
    void timeout();
};

class HyperspaceTCPStream : public HyperspaceStream
{
    Q_OBJECT
    Q_DISABLE_COPY(HyperspaceTCPStream)

public:
    explicit HyperspaceTCPStream(const QUrl& url, const QHash< QByteArray, QByteArray > &arguments, QObject* parent = nullptr);
    virtual ~HyperspaceTCPStream();

    virtual QIODevice *device() Q_DECL_OVERRIDE;
    virtual void close() Q_DECL_OVERRIDE;

private:
    QUrl m_url;
    QHash< QByteArray, QByteArray > m_arguments;
    QTcpSocket *m_socket;
};

class TCPTransport : public Hemera::DeveloperMode::Transport
{
    Q_OBJECT
    Q_DISABLE_COPY(TCPTransport)

public:
    explicit TCPTransport(const QString &id, QObject* parent = Q_NULLPTR);
    explicit TCPTransport(const QUrl &baseUrl, QObject* parent = Q_NULLPTR);

    virtual HyperspaceStream* streamRequest(const QString& relativeTarget,
                                            const QHash< QByteArray, QByteArray > &arguments = QHash< QByteArray, QByteArray >(),
                                            QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;
    virtual QUrl urlForRelativeTarget(const QString& relativeTarget) Q_DECL_OVERRIDE;

    virtual void startConnecting() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void checkHostIsAlive();

private:
    QUrl m_baseUrl;
    QTimer *m_discoveryTimer;
    QPointer< HyperDiscoveryClient > m_discoveryClient;
    QString m_id;
};
}
}

#endif // HEMERA_DEVELOPERMODE_TCPTRANSPORT_H
