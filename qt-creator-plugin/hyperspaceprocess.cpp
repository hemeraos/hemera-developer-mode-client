#include "hyperspaceprocess.h"

#include "hemeratarget.h"

#include <QtCore/QPointer>
#include <QtCore/QTimer>

#include <hemeradevelopermodecontroller.h>
#include <hemeradevelopermodetarget.h>
#include <hemeradevelopermodeshelloperation.h>

#include <utils/environment.h>
#include <utils/qtcassert.h>

namespace Hemera {
namespace Internal {

class HyperspaceProcess::Private
{
public:
    Private(HyperspaceProcess *q) : q(q), environment(Utils::Environment::systemEnvironment()), workingDirectory(QStringLiteral("/root")) {}

    HyperspaceProcess * const q;

    QPointer<Hemera::DeveloperMode::ShellOperation> operation;

    QString errorMessage;
    QTimer killTimer;
    QByteArray stdOut;
    QByteArray stdErr;
    int exitCode;
    Utils::Environment environment;
    QString workingDirectory;
};

HyperspaceProcess::HyperspaceProcess(const QSharedPointer<const ProjectExplorer::IDevice> &device, QObject *parent)
    : ProjectExplorer::DeviceProcess(device, parent)
    , d(new Private(this))
{
}

HyperspaceProcess::~HyperspaceProcess()
{
    qDebug() << "Deleting hyperspace process" << this;
    delete d;
}

void HyperspaceProcess::start(const QString &executable, const QStringList &arguments)
{
    QTC_ASSERT(d->operation.isNull(), return);

    d->errorMessage.clear();
    d->exitCode = -1;

    // Get the hold of the Hemera native device
    HemeraTarget::ConstPtr hemeraTarget = device().dynamicCast<const HemeraTarget>();
    if (hemeraTarget.isNull()) {
        qWarning() << "Could not retrieve a valid Hemera target!!";
        emit finished();
    }

    // Ensure the controller
    connect(hemeraTarget->target()->ensureDeveloperModeController(), &DeveloperMode::Operation::finished, this,
            [this, hemeraTarget, executable, arguments] (DeveloperMode::Operation *operation) {
        if (operation->isError()) {
            qWarning() << "Could not retrieve Developer mode Controller!";
            emit finished();
            return;
        }

        qDebug() << this;
        qDebug() << d;
        qDebug() << d->environment.toStringList() << d->workingDirectory;
        // d->environment.toStringList() crashes. The actual fuck??
        d->operation = hemeraTarget->target()->developerModeController()->executeShellCommand(executable, arguments, QStringList(), d->workingDirectory);

        connect(d->operation.data(), SIGNAL(processError(QProcess::ProcessError)), SLOT(handleProcessError(QProcess::ProcessError)));
        connect(d->operation.data(), SIGNAL(processStateChanged(QProcess::ProcessState)), SLOT(handleProcessStateChanged(QProcess::ProcessState)));
        connect(d->operation.data(), SIGNAL(stdoutMessage(QByteArray)), SLOT(handleStdout(QByteArray)));
        connect(d->operation.data(), SIGNAL(stderrMessage(QByteArray)), SLOT(handleStderr(QByteArray)));


        connect(d->operation.data(), &Hemera::DeveloperMode::ShellOperation::finished, [this] {
            if (!d->operation->isError()) {
                emit finished();
            }
        });
    });
}

void HyperspaceProcess::handleProcessStateChanged(QProcess::ProcessState newState)
{
    switch (newState) {
    case QProcess::Running:
        Q_EMIT started();
        break;
    default:
        break;
    }
}

void HyperspaceProcess::handleProcessError(QProcess::ProcessError pError)
{
    Q_EMIT error(pError);
}

void HyperspaceProcess::interrupt()
{
    if (!d->operation.isNull()) {
        d->operation->terminate();
    }
}

void HyperspaceProcess::terminate()
{
    if (!d->operation.isNull()) {
        d->operation->terminate();
    }
}

void HyperspaceProcess::kill()
{
    if (!d->operation.isNull()) {
        d->operation->kill();
    }
}

QProcess::ProcessState HyperspaceProcess::state() const
{
    if (!d->operation.isNull()) {
        if (d->operation->isFinished()) {
            return QProcess::Running;
        }
    }
    return QProcess::NotRunning;
}

QProcess::ExitStatus HyperspaceProcess::exitStatus() const
{
    return QProcess::NormalExit;
}

int HyperspaceProcess::exitCode() const
{
    return d->exitCode;
}

QString HyperspaceProcess::errorString() const
{
    return d->errorMessage;
}

Utils::Environment HyperspaceProcess::environment() const
{
    return d->environment;
}

void HyperspaceProcess::setEnvironment(const Utils::Environment &env)
{
    d->environment = env;
}

void HyperspaceProcess::setWorkingDirectory(const QString &workingDirectory)
{
    d->workingDirectory = workingDirectory;
}

QByteArray HyperspaceProcess::readAllStandardOutput()
{
    const QByteArray data = d->stdOut;
    d->stdOut.clear();
    return data;
}

QByteArray HyperspaceProcess::readAllStandardError()
{
    const QByteArray data = d->stdErr;
    d->stdErr.clear();
    return data;
}

void HyperspaceProcess::handleStdout(const QByteArray &message)
{
    d->stdOut += message;
    emit readyReadStandardOutput();
}

void HyperspaceProcess::handleStderr(const QByteArray &message)
{
    d->stdErr += message;
    emit readyReadStandardError();
}

qint64 HyperspaceProcess::write(const QByteArray &data)
{
    if (d->operation.isNull()) {
        return -1;
    }

    d->operation->writeStdin(data);

    return data.size();
}

}
}
