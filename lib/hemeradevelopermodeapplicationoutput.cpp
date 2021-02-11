#include "hemeradevelopermodeapplicationoutput.h"

#include "hemeradevelopermodeglobalobjects_p.h"
#include "hemeradevelopermodetarget.h"

#include "transports/hemeradevelopermodetransport.h"
#include "hemeradevelopermodehyperspacestream.h"

#include <qjsonstream.h>
#include <QtCore/QDebug>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

// TODO: We assume HTTP until Hyperspace Client comes and saves us all.

namespace Hemera {
namespace DeveloperMode {

class ApplicationOutput::Private
{
public:
    Private(ApplicationOutput *q) : q(q), valid(false), jsonStreamer(0), maxCacheSize(1000) {}

    ApplicationOutput * const q;

    bool valid;

    Target::Ptr target;
    QIODevice *device;
    QtAddOn::QtJsonStream::QJsonStream *jsonStreamer;
    int maxCacheSize;

    QMap< QDateTime, QJsonObject > messages;

    void dataFromDevice();
};

void ApplicationOutput::Private::dataFromDevice()
{
    while (jsonStreamer->messageAvailable()) {
        QJsonObject result = jsonStreamer->readMessage();
        // QJson has no concept of anything bigger than an int... This is such a disaster, really.
        QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(result.value(QStringLiteral("timestamp")).toString().toULongLong());
        result.remove(QStringLiteral("timestamp"));

        while (messages.size() >= maxCacheSize) {
            messages.erase(messages.begin());
        }

        messages.insert(timestamp, result);
        Q_EMIT q->newMessage(timestamp, result);
    }
}


ApplicationOutput::ApplicationOutput(const Target::Ptr &target, const QString& applicationId, QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
    d->target = target;

    // Set up stream request!
    QHash< QByteArray, QByteArray > arguments;
    arguments.insert("Gravity-Developer-Mode-Application-ID", applicationId.toLatin1());
    HyperspaceStream *stream = target->transport()->streamRequest(QStringLiteral("com.ispirata.Hemera.DeveloperMode/applicationoutput"), arguments, this);
    QObject::connect(stream, &HyperspaceStream::stateChanged, [this, stream] (QAbstractSocket::SocketState state) {
        if (state == QAbstractSocket::ConnectedState) {
            // We shall connect to the device
            d->device = stream->device();
            d->jsonStreamer = new QtAddOn::QtJsonStream::QJsonStream(d->device);
            d->jsonStreamer->setFormat(QtAddOn::QtJsonStream::FormatBSON);
            connect(d->jsonStreamer, SIGNAL(readyReadMessage()), this, SLOT(dataFromDevice()));
        }
    });

    // TODO: Error management and socket recovery
}

ApplicationOutput::~ApplicationOutput()
{
    if (d->jsonStreamer) {
        d->jsonStreamer->deleteLater();
    }
    delete d;
}

bool ApplicationOutput::isValid()
{
    return d->valid;
}

QMap< QDateTime, QJsonObject > ApplicationOutput::messages() const
{
    return d->messages;
}

void ApplicationOutput::setMaxMessageCacheSize(int maxCacheSize)
{
    d->maxCacheSize = maxCacheSize;
}


}
}

// For Q_PRIVATE_SLOT
#include "moc_hemeradevelopermodeapplicationoutput.cpp"
