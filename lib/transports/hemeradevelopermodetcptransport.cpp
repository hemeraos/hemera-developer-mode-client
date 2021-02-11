#include "hemeradevelopermodetcptransport.h"

#include "hemeradevelopermodeglobalobjects_p.h"
#include <hemeradevelopermodehyperdiscoveryclient.h>

#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QTcpSocket>

namespace Hemera {
namespace DeveloperMode {

QReplyTimeout::QReplyTimeout(QNetworkReply* reply, const int timeout)
    : QObject(reply)
{
    Q_ASSERT(reply);
    if (reply) {
        QTimer::singleShot(timeout, this, SLOT(timeout()));
    }
}

QReplyTimeout::~QReplyTimeout()
{
}

void QReplyTimeout::timeout()
{
    QNetworkReply* reply = static_cast<QNetworkReply*>(parent());
    if (reply->isRunning()) {
        reply->close();
    }
}


HyperspaceTCPStream::HyperspaceTCPStream(const QUrl& url, const QHash< QByteArray, QByteArray >& arguments, QObject* parent)
    : HyperspaceStream(parent)
    , m_url(url)
    , m_arguments(arguments)
{
    // Set up stream request!
    QNetworkRequest request(m_url);
    for (QHash< QByteArray, QByteArray >::const_iterator i = m_arguments.constBegin(); i != m_arguments.constEnd(); ++i) {
        request.setRawHeader(i.key(), i.value());
    }

    setState(QAbstractSocket::ConnectingState);

    QNetworkReply *reply = GlobalObjects::instance()->networkAccessManager()->sendCustomRequest(request, "STREAMREQUEST");
    connect(reply, &QNetworkReply::finished, [this, reply] {
        if (reply->error() != QNetworkReply::NoError) {
            Q_EMIT error(QAbstractSocket::OperationError);
            return;
        }

        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 202) {
            Q_EMIT error(QAbstractSocket::ConnectionRefusedError);
            return;
        }

        QByteArray secret = reply->rawHeader("Hyperspace-Stream-Secret");
        int port = reply->rawHeader("Hyperspace-Stream-TCP-Port").toInt();

        // Hyperstream power
        m_socket = new QTcpSocket(this);
        connect(m_socket, &QTcpSocket::stateChanged, [this] (QAbstractSocket::SocketState state) {
            if (state != QAbstractSocket::ConnectedState) {
                setState(state);
            }
        });
        connect(m_socket, &QTcpSocket::connected, [this, secret] {
            if (m_socket->write(secret) != secret.size()) {
                qWarning() << "Could not stream socket! Retrying...";
                QEventLoop e;
                QTimer::singleShot(100, &e, SLOT(quit()));
                e.exec();
            }
            setState(QAbstractSocket::ConnectedState);
        });

        m_socket->connectToHost(m_url.host(), port);
    });
}

HyperspaceTCPStream::~HyperspaceTCPStream()
{
}

void HyperspaceTCPStream::close()
{
    m_socket->disconnectFromHost();
    m_socket->close();
}

QIODevice *HyperspaceTCPStream::device()
{
    return m_socket;
}



TCPTransport::TCPTransport(const QUrl &url, QObject* parent)
    : Transport(parent)
{
    m_baseUrl = url;
    setNeedsKeepAliveCheck(true);
}

TCPTransport::TCPTransport(const QString &id, QObject* parent)
    : Transport(parent)
    , m_discoveryTimer(new QTimer(this))
    , m_id(id)
{
    m_discoveryClient = new HyperDiscoveryClient(this);
    connect(m_discoveryClient.data(), &HyperDiscoveryClient::capabilityDiscovered, [this, id] (QByteArray capability, const QHostAddress &address, int port, int ttl) {
        if (capability.right(capability.length() - 5) == id) {
            // Stop polling
            m_discoveryTimer->stop();

            // Check TTL, first of all.
            if (ttl <= 0) {
                // We lost him, Jim!
                setOnline(false);
                // Keep searching
                m_discoveryTimer->start(10000);
                return;
            }

            QUrl url;
            url.setScheme(QStringLiteral("https")); // FIXME: This needs to be get from the discovery!
            url.setHost(address.toString());
            url.setPort(port);
            if (m_baseUrl != url) {
                // Do it.
                m_baseUrl = url;
                checkHostIsAlive();
            }

            // We need to search again, when TTL expires.
            m_discoveryTimer->start(ttl * 1000);
        }
    });

    // Insist and persist
    m_discoveryTimer->setSingleShot(false);
    connect(m_discoveryTimer, &QTimer::timeout, [this] { m_discoveryClient->scanCapabilities(QList<QByteArray>() << ("hwid." + m_id).toLatin1()); });
}

void TCPTransport::startConnecting()
{
    if (m_discoveryClient.isNull()) {
        QTimer::singleShot(0, this, SLOT(checkHostIsAlive()));
    } else {
        // We put a 10000 ms interval here,
        m_discoveryTimer->start(10000);
        m_discoveryClient->scanCapabilities(QList<QByteArray>() << ("hwid." + m_id).toLatin1());
    }
}

void TCPTransport::checkHostIsAlive()
{
    // We craft a fake request, just so we know it's online.
    QNetworkRequest req(urlForRelativeTarget(QStringLiteral("/")));
    QNetworkReply *reply = GlobalObjects::instance()->networkAccessManager()->get(req);
    // Set a timeout on request of 2 seconds
    new QReplyTimeout(reply, 2000);

    QObject::connect(reply, &QNetworkReply::finished, [this, reply] {
        if (reply->error() != QNetworkReply::NoError && reply->error() != QNetworkReply::ContentNotFoundError) {
            QTimer::singleShot(2000, this, SLOT(checkHostIsAlive()));
            setOnline(false);
        } else {
            setOnline(true);
        }
        reply->deleteLater();
    });
}

HyperspaceStream* TCPTransport::streamRequest(const QString& relativeTarget, const QHash< QByteArray, QByteArray > &arguments, QObject *parent)
{
    return new HyperspaceTCPStream(urlForRelativeTarget(relativeTarget), arguments, parent);
}

QUrl TCPTransport::urlForRelativeTarget(const QString& relativeTarget)
{
    QUrl url = m_baseUrl;

    // Circumvent tremendous QUrl bug!!!!
    QString realRelativeTarget = relativeTarget;
    if (!realRelativeTarget.startsWith(QLatin1Char('/'))) {
        realRelativeTarget.prepend(QLatin1Char('/'));
    }

    url.setPath(realRelativeTarget);

    return url;
}


}
}
