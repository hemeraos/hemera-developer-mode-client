#include <QtCore/QByteArray>
#include <QtCore/QIODevice>

class http_parser;
class http_parser_settings;

class HttpServerConnection : public QObject
{
    Q_OBJECT

    public:

        HttpServerConnection(QIODevice *parent = nullptr);
        virtual ~HttpServerConnection();

    public Q_SLOT:
        void readyRead();

    protected:
        void setParser(http_parser *parser, http_parser_settings *settings);

        virtual int onUrl(const QByteArray &url);
        virtual int onStatus(const QByteArray &status);
        virtual int onHeaderField(const QByteArray &headerField);
        virtual int onHeaderValue(const QByteArray &headerValue);
        virtual int onBody(const QByteArray &body);
        virtual int onMessageBegin();
        virtual int onHeadersComplete();
        virtual int onMessageComplete();

    private:
        http_parser *m_parser;
        http_parser_settings *m_parserSettings;

        friend class HttpServer;
};
