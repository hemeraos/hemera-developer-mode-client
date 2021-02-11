#include "HttpServerConnection.h"

#include <QtCore/QDebug>

#include "http_parser.h"

HttpServerConnection::HttpServerConnection(QIODevice *parent)
    : QObject(parent)
    , m_parser(nullptr)
    , m_parserSettings(nullptr)
{
}

HttpServerConnection::~HttpServerConnection()
{
    delete m_parser;
}

void HttpServerConnection::readyRead()
{
    QByteArray data = static_cast<QIODevice *>(parent())->readAll();
    int read = data.count();
    int parsed = http_parser_execute(m_parser, m_parserSettings, data.constData(), read);
    if (read != parsed) {
        qDebug() << "data lost" << read << parsed;
    }
}

void HttpServerConnection::setParser(http_parser *parser, http_parser_settings *settings)
{
    m_parser = parser;
    m_parserSettings = settings;
}

int HttpServerConnection::onUrl(const QByteArray &url)
{
    qDebug() << "url: " << url;
    return 0;
}

int HttpServerConnection::onStatus(const QByteArray &status)
{
    qDebug() << "status: " << status;
    return 0;
}

int HttpServerConnection::onHeaderField(const QByteArray &headerField)
{
    qDebug() << "header field: " << headerField;
    return 0;
}

int HttpServerConnection::onHeaderValue(const QByteArray &headerValue)
{
    qDebug() << "header value: " << headerValue;
    return 0;
}

int HttpServerConnection::onBody(const QByteArray &body)
{
    qDebug() << "body: " << body;
    return 0;
}

int HttpServerConnection::onMessageBegin()
{
    qDebug() << "message begin";
    return 0;
}

int HttpServerConnection::onHeadersComplete()
{
    qDebug() << "headers complete";
    return 0;
}

int HttpServerConnection::onMessageComplete()
{
    qDebug() << "message complete";
    return 0;
}
