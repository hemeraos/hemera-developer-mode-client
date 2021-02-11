#include "HttpTCPServer.h"

#include "HttpServerConnection.h"

#define CONNECTION_TIMEOUT 15000

#include <QtCore/QTimer>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>


HttpTCPServer::HttpTCPServer(QObject *parent)
    : HttpServer(parent)
    , m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, &HttpTCPServer::clientConnected);
}

HttpTCPServer::~HttpTCPServer()
{
}

bool HttpTCPServer::listen(const QHostAddress &address, quint16 port)
{
    return m_server->listen(address, port);
}


HttpServerConnection *HttpTCPServer::createServerConnection(QTcpSocket *socket)
{
    return new HttpServerConnection(socket);
}

void HttpTCPServer::clientConnected()
{
    QTcpServer *server = qobject_cast<QTcpServer *>(sender());
    QTcpSocket *socket = server->nextPendingConnection();

    HttpServerConnection *conn = createServerConnection(socket);
    setupServerConnection(conn);

    // Parenting to the socket. This way, when the socket dies, the timer does too.
    QTimer *timeoutTimer = new QTimer(socket);
    timeoutTimer->setSingleShot(true);
    timeoutTimer->setInterval(CONNECTION_TIMEOUT);
    timeoutTimer->start();

    // Timeout
    connect(timeoutTimer, &QTimer::timeout, socket, &QTcpSocket::close);

    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    connect(socket, &QTcpSocket::readyRead, conn, &HttpServerConnection::readyRead);
}
