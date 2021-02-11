#ifndef HEMERA_DEVELOPERMODE_APPLICATIONOUTPUT_H
#define HEMERA_DEVELOPERMODE_APPLICATIONOUTPUT_H

#include "hemeradevelopermodeexport.h"

#include "hemeradevelopermodetarget.h"

#include <QtCore/QDateTime>
#include <QtCore/QMap>
#include <QtCore/QObject>

class QJsonObject;

namespace Hemera {
namespace DeveloperMode {

class Target;

class HemeraDeveloperModeClient_EXPORT ApplicationOutput : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ApplicationOutput)

    Q_PRIVATE_SLOT(d, void dataFromDevice())

public:
    explicit ApplicationOutput(const Target::Ptr &target, const QString &applicationId, QObject* parent = 0);
    virtual ~ApplicationOutput();

    bool isValid();

    void setMaxMessageCacheSize(int maxCacheSize = 1000);

    QMap< QDateTime, QJsonObject > messages() const;

Q_SIGNALS:
    void connected();
    void disconnected();

    void newMessage(const QDateTime &timestamp, const QJsonObject &message);

private:
    class Private;
    Private * const d;
};
}
}

#endif // HEMERA_DEVELOPERMODE_APPLICATIONOUTPUT_H
