#ifndef HEMERA_DEVELOPERMODE_CONTROLLER_P_H
#define HEMERA_DEVELOPERMODE_CONTROLLER_P_H

#include <hemeradevelopermodecontroller.h>

#include "hemeradevelopermodeglobalobjects_p.h"
#include "hemeradevelopermodehyperspacestream.h"

#include <QtCore/QJsonObject>

namespace QtAddOn {
namespace QtJsonStream {
class QJsonStream;
}
}

namespace Hemera {
namespace DeveloperMode {

class Controller::Private {
public:
    Private(Controller *q) : q(q), connected(true), nam(GlobalObjects::instance()->networkAccessManager()) {}

    Controller * const q;

    bool connected;

    HyperspaceStream *stream;
    Target::Ptr target;
    QNetworkAccessManager *nam;
    QIODevice *device;
    QtAddOn::QtJsonStream::QJsonStream *jsonStreamer;
    QHash< QString, QString > expectReplyType;

    QHash< QString, Status > statuses;

    QHash< QString, DeployOperation* > pendingDeployments;

    QHash< QString, ShellOperation* > shellOperations;

    QByteArray partialDeviceData;

    QString sendRequest(const QString &command, const QString &star, const QJsonObject &data = QJsonObject());
    void dataFromDevice();

    void parseNotification(const QJsonObject &data);
    void parseReply(const QJsonObject &data);

    void setStarStatus(const QString &star, bool active);
};

}
}

#endif // HEMERA_DEVELOPERMODE_CONTROLLER_P_H
