#include "HttpServer.h"

#include "HttpServerConnection.h"
#include "http_parser.h"

HttpServer::HttpServer(QObject *parent)
    : QObject(parent)
    , m_parserSettings(new http_parser_settings)
{
    // Set up our global parser settings
    m_parserSettings->on_url = onUrl;
    m_parserSettings->on_header_field = onHeaderField;
    m_parserSettings->on_body = onBody;
    m_parserSettings->on_header_value = onHeaderValue;
    m_parserSettings->on_headers_complete = onHeadersComplete;
    m_parserSettings->on_message_begin = onMessageBegin;
    m_parserSettings->on_message_complete = onMessageComplete;
    m_parserSettings->on_status = onStatus;
}

HttpServer::~HttpServer()
{
    delete m_parserSettings;
}

void HttpServer::setupServerConnection(HttpServerConnection *connection)
{
    http_parser *parser = new http_parser();
    http_parser_init(parser, HTTP_BOTH);
    parser->data = connection;
    connection->setParser(parser, m_parserSettings);
}

int HttpServer::onUrl(http_parser *parser, const char *url, size_t urlLen)
{
    return static_cast<HttpServerConnection *>(parser->data)->onUrl(QByteArray::fromRawData(url, urlLen));
}

int HttpServer::onStatus(http_parser *parser, const char *status, size_t statusLen)
{
    return static_cast<HttpServerConnection *>(parser->data)->onStatus(QByteArray::fromRawData(status, statusLen));
}

int HttpServer::onHeaderField(http_parser *parser, const char *headerField, size_t headerLen)
{
    return static_cast<HttpServerConnection *>(parser->data)->onHeaderField(QByteArray::fromRawData(headerField, headerLen));
}

int HttpServer::onHeaderValue(http_parser *parser, const char *headerValue, size_t valueLen)
{
    return static_cast<HttpServerConnection *>(parser->data)->onHeaderValue(QByteArray::fromRawData(headerValue, valueLen));
}

int HttpServer::onBody(http_parser *parser, const char *body, size_t bodyLen)
{
    return static_cast<HttpServerConnection *>(parser->data)->onBody(QByteArray::fromRawData(body, bodyLen));
}

int HttpServer::onMessageBegin(http_parser *parser)
{
    return static_cast<HttpServerConnection *>(parser->data)->onMessageBegin();
}

int HttpServer::onHeadersComplete(http_parser *parser)
{
    return static_cast<HttpServerConnection *>(parser->data)->onHeadersComplete();
}

int HttpServer::onMessageComplete(http_parser *parser)
{
    return static_cast<HttpServerConnection *>(parser->data)->onMessageComplete();
}
