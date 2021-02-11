#include "hemeradevelopermodeshelloperation.h"

#include "hemeradevelopermodeglobalobjects_p.h"
#include "hemeradevelopermodetarget.h"
#include "hemeradevelopermodecontroller.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtCore/QJsonObject>

#include <QtNetwork/QHostInfo>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace Hemera {
namespace DeveloperMode {

class ShellOperation::Private {
public:
    Private() : elapsedTimer(new QElapsedTimer) {}

    QElapsedTimer *elapsedTimer;
    QString requestId;
    Controller *controller;
};

ShellOperation::ShellOperation(const QString &requestId, Controller *parent)
    : Operation(parent)
    , d(new Private)
{
    d->requestId = requestId;
    d->controller = parent;
}

ShellOperation::~ShellOperation()
{
    delete d->elapsedTimer;
    delete d;
}

void ShellOperation::startImpl()
{
    // Start timer
    d->elapsedTimer->start();
}

void ShellOperation::kill()
{
    QJsonObject data;
    data.insert(QStringLiteral("process"), d->requestId);
    data.insert(QStringLiteral("action"), QStringLiteral("kill"));
    d->controller->sendRequest(QStringLiteral("shell-control"), QString(), data);
}

void ShellOperation::terminate()
{
    QJsonObject data;
    data.insert(QStringLiteral("process"), d->requestId);
    data.insert(QStringLiteral("action"), QStringLiteral("terminate"));
    d->controller->sendRequest(QStringLiteral("shell-control"), QString(), data);
}

void ShellOperation::interrupt()
{
    QJsonObject data;
    data.insert(QStringLiteral("process"), d->requestId);
    data.insert(QStringLiteral("action"), QStringLiteral("interrupt"));
    d->controller->sendRequest(QStringLiteral("shell-control"), QString(), data);
}


void ShellOperation::writeStdin(const QByteArray &payload)
{
    QJsonObject data;
    data.insert(QStringLiteral("process"), d->requestId);
    data.insert(QStringLiteral("action"), QStringLiteral("write"));
    data.insert(QStringLiteral("payload"), QString::fromLatin1(payload));
    d->controller->sendRequest(QStringLiteral("shell-control"), QString(), data);
}

void ShellOperation::sendStdinEOF()
{
    QJsonObject data;
    data.insert(QStringLiteral("process"), d->requestId);
    data.insert(QStringLiteral("action"), QStringLiteral("stdinEOF"));
    d->controller->sendRequest(QStringLiteral("shell-control"), QString(), data);
}



}
}

#include "moc_hemeradevelopermodeshelloperation.cpp"
