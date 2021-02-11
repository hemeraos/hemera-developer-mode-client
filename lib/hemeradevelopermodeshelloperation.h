#ifndef HEMERA_DEVELOPERMODE_SHELLOPERATION_H
#define HEMERA_DEVELOPERMODE_SHELLOPERATION_H

#include "hemeradevelopermodeoperation.h"

#include "hemeradevelopermodeexport.h"

#include <QtCore/QObject>
#include <QtCore/QProcess>

class QIODevice;
namespace Hemera {
namespace DeveloperMode {

class Target;

class Controller;

class HemeraDeveloperModeClient_EXPORT ShellOperation : public Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(ShellOperation)

public:
    virtual ~ShellOperation();

public Q_SLOTS:
    void interrupt();
    void terminate();
    void kill();
    void writeStdin(const QByteArray &message);
    void sendStdinEOF();

protected Q_SLOTS:
    virtual void startImpl();

Q_SIGNALS:
    void stdoutMessage(const QByteArray &message);
    void stderrMessage(const QByteArray &message);

    void processStateChanged(QProcess::ProcessState newState);
    void processError(QProcess::ProcessError error);

private:
    explicit ShellOperation(const QString &processId, Controller *parent);

    class Private;
    Private * const d;

    friend class Controller;
};

}
}

#endif // HEMERA_DEVELOPERMODE_SHELLOPERATION_H
