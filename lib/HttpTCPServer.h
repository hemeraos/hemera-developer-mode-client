#ifndef _HTTPSERVER_HTTPTCPSERVER_H_
#define _HTTPSERVER_HTTPTCPSERVER_H_

#include "HttpServer.h"

#include <QtNetwork/QHostAddress>

class QTcpServer;
class QTcpSocket;

class HttpServerConnection;

class HttpTCPServer : public HttpServer
{
    Q_OBJECT

    public:
        HttpTCPServer(QObject *parent = nullptr);
        ~HttpTCPServer();

        bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);

    protected:
        virtual HttpServerConnection *createServerConnection(QTcpSocket *parentConnectionSocket);

    private Q_SLOTS:
        void clientConnected();

    private:
        QTcpServer *m_server;
};

#endif
