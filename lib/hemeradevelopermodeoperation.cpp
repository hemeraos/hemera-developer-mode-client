#include "hemeradevelopermodeoperation.h"

#include <QtCore/QEventLoop>
#include <QtCore/QLoggingCategory>
#include <QtCore/QTimer>

Q_LOGGING_CATEGORY(LOG_HEMERA_OPERATION, "Hemera::DeveloperMode::Operation")

namespace Hemera {
namespace DeveloperMode {

class Operation::Private
{
public:
    Private(Operation *q) : q(q), finished(false) {}

    Operation *q;

    QString errorName;
    QString errorMessage;
    bool finished;

    // Private slots
    void emitFinished();
};

void Operation::Private::emitFinished()
{
    Q_ASSERT(finished);
    Q_EMIT q->finished(q);
    // Delay deletion of 2 seconds to be safe
    QTimer::singleShot(2000, q, SLOT(deleteLater()));
}


Operation::Operation(QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
    QTimer::singleShot(0, this, SLOT(startImpl()));
}

Operation::~Operation()
{
    if (!isFinished()) {
        qCWarning(LOG_HEMERA_OPERATION) << "The operation is being deleted, but is not finished yet.";
    }

    delete d;
}

QString Operation::errorMessage() const
{
    return d->errorMessage;
}

QString Operation::errorName() const
{
    return d->errorName;
}

bool Operation::isError() const
{
    return (d->finished && !d->errorName.isEmpty());
}

bool Operation::isFinished() const
{
    return d->finished;
}

bool Operation::isValid() const
{
    return (d->finished && d->errorName.isEmpty());
}

void Operation::setFinished()
{
    if (d->finished) {
        if (!d->errorName.isEmpty()) {
            qCWarning(LOG_HEMERA_OPERATION) << this << "trying to finish with success, but already"
                " failed with" << d->errorName << ":" << d->errorMessage;
        } else {
            qCWarning(LOG_HEMERA_OPERATION) << this << "trying to finish with success, but already"
                " succeeded";
        }
        return;
    }

    d->finished = true;
    Q_ASSERT(isValid());
    QTimer::singleShot(0, this, SLOT(emitFinished()));
}

void Operation::setFinishedWithError(const QString& name, const QString& message)
{
    if (d->finished) {
        if (!d->errorName.isEmpty()) {
            qCWarning(LOG_HEMERA_OPERATION) << this << "trying to fail with" << name <<
                "but already failed with" << errorName() << ":" <<
                errorMessage();
        } else {
            qCWarning(LOG_HEMERA_OPERATION) << this << "trying to fail with" << name <<
                "but already succeeded";
        }
        return;
    }

    if (name.isEmpty()) {
        qCWarning(LOG_HEMERA_OPERATION) << this << "should be given a non-empty error name";
        d->errorName = QStringLiteral("no.error");
    } else {
        d->errorName = name;
    }

    d->errorMessage = message;
    d->finished = true;
    Q_ASSERT(isError());
    QTimer::singleShot(0, this, SLOT(emitFinished()));
}

bool Operation::synchronize(int timeout)
{
    if (isFinished()) {
        return !isError();
    }

    QEventLoop e;
    connect(this, &Hemera::DeveloperMode::Operation::finished, &e, &QEventLoop::quit);

    if (timeout > 0) {
        QTimer::singleShot(timeout * 1000, &e, SLOT(quit()));
    }

    e.exec();

    if (!isFinished()) {
        qCWarning(LOG_HEMERA_OPERATION) << this << " timed out while trying to synchronize.";
        return false;
    }

    return !isError();
}

}
}

#include "moc_hemeradevelopermodeoperation.cpp"
