#ifndef _HTTPSERVER_HTTPSERVER_H_
#define _HTTPSERVER_HTTPSERVER_H_

#include <QtCore/QObject>

class HttpServerConnection;

struct http_parser;
struct http_parser_settings;

class HttpServer : public QObject
{
    Q_OBJECT

    public:
        HttpServer(QObject *parent = nullptr);
        ~HttpServer();

    protected:
        void setupServerConnection(HttpServerConnection *connection);

    private:
        http_parser_settings *m_parserSettings;

        static int onUrl(http_parser *parser, const char *url, size_t urlLen);
        static int onStatus(http_parser *parser, const char *status, size_t statusLen);
        static int onHeaderField(http_parser *parser, const char *headerField, size_t headerLen);
        static int onHeaderValue(http_parser *parser, const char *headerValue, size_t valueLen);
        static int onBody(http_parser *parser, const char *body, size_t bodyLen);
        static int onMessageBegin(http_parser *parser);
        static int onHeadersComplete(http_parser *parser);
        static int onMessageComplete(http_parser *parser);
};

#endif
