/*
 *
 */

#include "hemeradevelopermodecontroller_p.h"

#include "hemeradevelopermodetarget.h"
#include "hemeradevelopermodedeployoperation.h"
#include "hemeradevelopermodestar.h"
#include "hemeradevelopermodeshelloperation.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QEventLoop>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QUuid>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <qjsonstream.h>

#define HEMERA_DEV_MODE_URL "/com.ispirata.Hemera.DeveloperMode/"

#define HEMERA_VM_BASE_HOME_PATH QStringLiteral("/home/developer/")

namespace Hemera {
namespace DeveloperMode {

void Controller::Private::setStarStatus(const QString& star, bool active)
{
    statuses.insert(star, active ? Status::Running : Status::Stopped);
    Q_EMIT q->statusChanged(star, statuses.value(star));
}

QString Controller::Private::sendRequest(const QString& command, const QString& star, const QJsonObject& data)
{
    QJsonObject ret;
    ret.insert(QStringLiteral("command"), command);
    QString requestId = QUuid::createUuid().toString();
    ret.insert(QStringLiteral("requestId"), requestId);
    expectReplyType.insert(requestId, command);

    if (!star.isEmpty()) {
        ret.insert(QStringLiteral("star"), star);
    }
    if (!data.isEmpty()) {
        ret.insert(QStringLiteral("data"), data);
    }

    if (jsonStreamer->send(ret)) {
        return requestId;
    } else {
        return QString();
    }
}

void Controller::Private::parseNotification(const QJsonObject& data)
{
    // Is it a type notification?
    if (data.contains(QStringLiteral("type"))) {
        QString type = data.value(QStringLiteral("type")).toString();
        if (type == QStringLiteral("packageinstall")) {
            // Retrieve DeployOperation
            DeployOperation *operation = pendingDeployments.take(data.value(QStringLiteral("file")).toString());
            if (!operation) {
                qWarning() << "Got a notification for an unknown deployment, something is not right.";
            }

            if (data.value(QStringLiteral("success")).toBool()) {
                operation->setFinished();
            } else {
                operation->setFinishedWithError(data.value(QStringLiteral("errorName")).toString(),
                                                data.value(QStringLiteral("errorString")).toString());
            }
        } else if (type == QStringLiteral("processOutput") && shellOperations.contains(data.value(QStringLiteral("process")).toString())) {
            ShellOperation *operation = shellOperations.value(data.value(QStringLiteral("process")).toString());
            if (data.value(QStringLiteral("channel")).toString() == QStringLiteral("stderr")) {
                Q_EMIT operation->stderrMessage(QByteArray::fromBase64(data.value(QStringLiteral("message")).toString().toLatin1()));
            } else {
                Q_EMIT operation->stdoutMessage(QByteArray::fromBase64(data.value(QStringLiteral("message")).toString().toLatin1()));
            }
        } else if (type == QStringLiteral("processState") && shellOperations.contains(data.value(QStringLiteral("process")).toString())) {
            ShellOperation *operation = shellOperations.value(data.value(QStringLiteral("process")).toString());
            Q_EMIT operation->processStateChanged(static_cast<QProcess::ProcessState>(data.value(QStringLiteral("state")).toInt()));
        } else if (type == QStringLiteral("processError") && shellOperations.contains(data.value(QStringLiteral("process")).toString())) {
            ShellOperation *operation = shellOperations.value(data.value(QStringLiteral("process")).toString());
            Q_EMIT operation->processError(static_cast<QProcess::ProcessError>(data.value(QStringLiteral("error")).toInt()));
        }
    } else {
        // All keys are the stars for this notification.
        for (QJsonObject::const_iterator i = data.constBegin(); i != data.constEnd(); ++i) {
            QJsonObject starData = i.value().toObject();
            if (starData.contains(QStringLiteral("active"))) {
                setStarStatus(i.key(), starData.value(QStringLiteral("active")).toBool());
            }

            // We should pass this over to the star object in any case.
            Star *star = target->star(i.key());
            if (star) {
                // It's a Q_PRIVATE_SLOT, so we have to do it this way.
                QMetaObject::invokeMethod(star, "refreshInfo");
            }
        }
    }
}

void Controller::Private::parseReply(const QJsonObject& data)
{
    QString requestId = data.value(QStringLiteral("requestId")).toString();
    bool success = data.value(QStringLiteral("success")).toBool();

    QString replyType = expectReplyType.take(requestId);
    if (replyType.isEmpty()) {
        qWarning() << "Request id" << requestId << "had nothing associated to it!!";
        return;
    }

    if (!success && replyType != QStringLiteral("shell")) {
        // Get error and stream it
        Q_EMIT q->error(data.value(QStringLiteral("errorName")).toString(), data.value(QStringLiteral("errorMessage")).toString());
        return;
    }

    QJsonObject replyData = data.value("data").toObject();

    if (replyType == QStringLiteral("status")) {
        for (QJsonObject::const_iterator i = replyData.constBegin(); i != replyData.constEnd(); ++i) {
            setStarStatus(i.key(), i.value().toBool());
        }
    } else if (replyType == QStringLiteral("shell")) {
        ShellOperation *operation = shellOperations.take(requestId);
        if (!operation) {
            qWarning() << "Got a shell reply, but no such shell operation is going on!!";
            return;
        }

        if (success) {
            operation->setFinished();
        } else {
            operation->setFinishedWithError(data.value(QStringLiteral("errorName")).toString(), data.value(QStringLiteral("errorMessage")).toString());
        }
    }
}

void Controller::Private::dataFromDevice()
{
    while (jsonStreamer->messageAvailable()) {
        QJsonObject result = jsonStreamer->readMessage();
        QString type = result.value(QStringLiteral("type")).toString();

        if (type == QStringLiteral("notify")) {
            parseNotification(result.value(QStringLiteral("data")).toObject());
        } else if (type == QStringLiteral("reply")) {
            parseReply(result);
        } else {
            qWarning() << "Unknown message type" << type << result << jsonStreamer->lastError();
        }
    }
}


Controller::Controller(const Target::Ptr &parent, HyperspaceStream* stream)
    : QObject(parent.data())
    , d(new Private(this))
{
    d->stream = stream;
    d->target = parent;
    // We assume the stream is already connected (it has to be!)
    d->device = stream->device();
    d->jsonStreamer = new QtAddOn::QtJsonStream::QJsonStream(d->device);
    d->jsonStreamer->setFormat(QtAddOn::QtJsonStream::FormatBSON);

    // We shall connect to the device
    connect(d->jsonStreamer, SIGNAL(readyReadMessage()), this, SLOT(dataFromDevice()));

    // Enquire the device about the status.
    d->sendRequest(QStringLiteral("status"), QString());

    // Shall we die...
    connect(stream, &HyperspaceStream::stateChanged, this, [this] (QAbstractSocket::SocketState state) {
        if ((state == QAbstractSocket::UnconnectedState || state == QAbstractSocket::ClosingState) && d->connected) {
            // We lost him, Jim!
            d->connected = false;
            Q_EMIT disconnected();

            // Clean up...
            for (QHash<QString, ShellOperation*>::const_iterator i = d->shellOperations.constBegin(); i != d->shellOperations.constEnd(); ++i) {
                i.value()->setFinishedWithError(QStringLiteral("Connection.Broken"), tr("The connection to the target has been severed!"));
            }
        }
    }, Qt::QueuedConnection);
    connect(this, &QObject::destroyed, [stream] {
        // Kill!
        stream->close();
        stream->deleteLater();
    });
}

Controller::~Controller()
{
    d->jsonStreamer->deleteLater();
    delete d;
}

void Controller::release()
{
    d->stream->close();
}

bool Controller::isValid() const
{
    return d->connected;
}

QHash< QString, Controller::Status > Controller::statuses() const
{
    return d->statuses;
}

Controller::Status Controller::statusOf(const QString& star) const
{
    return d->statuses.value(star, Status::Invalid);
}

void Controller::startSimple(const QString& star, const QString& application, bool withDebug,
                             const QString& debugPrefix, const QString& debugSuffix)
{
    QJsonObject data;
    data.insert(QStringLiteral("mode"), QStringLiteral("simple"));
    data.insert(QStringLiteral("application"), application);
    data.insert(QStringLiteral("debug"), withDebug);
    if (withDebug) {
        data.insert(QStringLiteral("debugPrefix"), debugPrefix);
        data.insert(QStringLiteral("debugSuffix"), debugSuffix);
    }

    d->sendRequest(QStringLiteral("start"), star, data);
}

void Controller::startAdvanced(const QString& star, const QString& applications, Features features,
                               bool withDebug, const QString& debugPrefix, const QString& debugSuffix)
{
    QJsonObject data;
    data.insert(QStringLiteral("mode"), QStringLiteral("advanced"));
    data.insert(QStringLiteral("applications"), applications);
    data.insert(QStringLiteral("features"), static_cast<int>(features));
    data.insert(QStringLiteral("debug"), withDebug);
    if (withDebug) {
        data.insert(QStringLiteral("debugPrefix"), debugPrefix);
        data.insert(QStringLiteral("debugSuffix"), debugSuffix);
    }

    d->sendRequest(QStringLiteral("start"), star, data);
}

void Controller::stop(const QString& star)
{
    d->sendRequest(QStringLiteral("stop"), star);
}

DeployOperation *Controller::deployPackage(const QString& filePath)
{
    QString fileName = QFileInfo(filePath).fileName();

    if (!d->pendingDeployments.contains(fileName)) {
        d->pendingDeployments.insert(fileName, new DeployOperation(filePath, d->target, this));
    }

    return d->pendingDeployments.value(fileName);
}

QString Controller::localPathToVm(const QDir &localDirectory)
{
    if (!localDirectory.exists()) {
        return QString();
    }

    QString path = localDirectory.absolutePath();
    QString homePath = QDir::homePath();

    if (!path.startsWith(homePath)) {
        return QString();
    }

    // Create our relative path
    path.remove(0, homePath.size() + 1);

    return QDir::cleanPath(path);
}

ShellOperation* Controller::executeShellCommand(const QString& program, const QStringList& arguments, const QStringList &environment, const QString& workingDirectory)
{
    QJsonObject data;
    data.insert(QStringLiteral("program"), program);
    data.insert(QStringLiteral("workingDirectory"), HEMERA_VM_BASE_HOME_PATH + workingDirectory);
    if (!arguments.isEmpty()) {
        data.insert(QStringLiteral("arguments"), QJsonArray::fromStringList(arguments));
    }
    if (!environment.isEmpty()) {
        data.insert(QStringLiteral("environment"), QJsonArray::fromStringList(environment));
    }

    QString requestId = d->sendRequest(QStringLiteral("shell"), QString(), data);

    // Send the request and create the operation right after.
    ShellOperation *operation = new ShellOperation(requestId, this);
    d->shellOperations.insert(requestId, operation);

    connect(operation, &ShellOperation::finished, [this, requestId] {
        d->shellOperations.remove(requestId);
    });

    return operation;
}

QString Controller::sendRequest(const QString& command, const QString& star, const QJsonObject& data)
{
    return d->sendRequest(command, star, data);
}

}
}

// For Q_PRIVATE_SLOT
#include "moc_hemeradevelopermodecontroller.cpp"
