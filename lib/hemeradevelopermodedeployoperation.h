#ifndef HEMERA_DEVELOPERMODE_DEPLOYOPERATION_H
#define HEMERA_DEVELOPERMODE_DEPLOYOPERATION_H

#include "hemeradevelopermodeoperation.h"

#include "hemeradevelopermodetarget.h"

#include "hemeradevelopermodeexport.h"

#include <QtCore/QObject>

namespace Hemera {
namespace DeveloperMode {

class Target;

class Controller;

class HemeraDeveloperModeClient_EXPORT DeployOperation : public Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(DeployOperation)

public:
    virtual ~DeployOperation();

protected Q_SLOTS:
    virtual void startImpl();

Q_SIGNALS:
    void progress(quint64 bytesUploaded, quint64 totalBytes, quint64 rate);
    void finished(Hemera::DeveloperMode::DeployOperation *operation);

private:
    explicit DeployOperation(const QString &file, const Target::Ptr &target, QObject* parent = 0);

    class Private;
    Private * const d;

    friend class Controller;
};

}
}

#endif // HEMERA_DEVELOPERMODE_DEPLOYOPERATION_H
