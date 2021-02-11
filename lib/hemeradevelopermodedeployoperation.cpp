#include "hemeradevelopermodedeployoperation.h"

#include "hemeradevelopermodeglobalobjects_p.h"
#include "hemeradevelopermodetarget.h"

#include "transports/hemeradevelopermodetransport.h"
#include "hemeradevelopermodehyperspacestream.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>

#include <QtNetwork/QHostInfo>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#define FILE_CHUNK_SIZE 65536

namespace Hemera {
namespace DeveloperMode {

class DeployOperation::Private {
public:
    // It starts at -38 as the exchanged secret does not count
    Private() : uploadedSize(-38), elapsedTimer(new QElapsedTimer) {}

    Target::Ptr target;

    QString filename;

    QFileInfo fileInfo;
    qint64 uploadedSize;
    QElapsedTimer *elapsedTimer;
};

DeployOperation::DeployOperation(const QString& file, const Target::Ptr &target, QObject* parent)
    : Operation(parent)
    , d(new Private)
{
    d->filename = file;
    d->target = target;
}

DeployOperation::~DeployOperation()
{
    delete d->elapsedTimer;
    delete d;
}

void DeployOperation::startImpl()
{
    // Gather info now
    d->fileInfo = QFileInfo(d->filename);

    // Verify the file exists, first
    if (!d->fileInfo.exists()) {
        setFinishedWithError(QStringLiteral("File not found"), QStringLiteral("Could not read from file!"));
        return;
    }

    if (d->fileInfo.size() <= 0) {
        setFinishedWithError(QStringLiteral("File cannot be fetched"), QStringLiteral("Could not read from file!"));
        return;
    }

    // We can start the negotiation.
    // Set up stream request!
    QHash< QByteArray, QByteArray > arguments;
    arguments.insert("Hyperspace-Stream-File-Name", d->fileInfo.fileName().toLatin1());
    QByteArray fileSize;
    fileSize.setNum(d->fileInfo.size());
    arguments.insert("Hyperspace-Stream-File-Size", fileSize);

    HyperspaceStream *stream = d->target->transport()->streamRequest(QStringLiteral("com.ispirata.Hemera.DeveloperMode/packageupload"), arguments, this);
    QObject::connect(stream, &HyperspaceStream::stateChanged, [this, stream] (QAbstractSocket::SocketState state) {
        if (state == QAbstractSocket::ConnectedState) {
            // We shall connect to the device
            QIODevice *device = stream->device();
            d->elapsedTimer->start();

            connect(device, &QIODevice::bytesWritten, [this] (qint64 bytes) {
                d->uploadedSize += bytes;
                Q_EMIT progress(d->uploadedSize, d->fileInfo.size(), d->elapsedTimer->elapsed() > 1000 ? d->uploadedSize / d->elapsedTimer->elapsed() : 0);
            });

            // Now we're ready to blast. We read/write the file in chunks.
            QFile file(d->filename);
            file.open(QIODevice::ReadOnly);
            while (!file.atEnd()) {
                QByteArray data = file.read(FILE_CHUNK_SIZE);
                device->write(data);

                // Event loop run for me.
                QCoreApplication::processEvents();
            }

            // Done. We wait for other signals.
        }
    });

    // TODO: Error management
}

}
}

#include "moc_hemeradevelopermodedeployoperation.cpp"
