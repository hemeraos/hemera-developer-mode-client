#ifndef HEMERA_DEVELOPERMODE_OPERATION_H
#define HEMERA_DEVELOPERMODE_OPERATION_H

#include <QtCore/QObject>

#include "hemeradevelopermodeexport.h"

namespace Hemera {
namespace DeveloperMode {

class HemeraDeveloperModeClient_EXPORT Operation : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Operation)

    Q_PRIVATE_SLOT(d, void emitFinished())

public:
    /**
     * Default destructor
     */
    virtual ~Operation();

    /**
     * Returns whether this operation has finished.
     *
     * @returns @p true if the Operation has finished, @p false if not.
     */
    bool isFinished() const;

    /**
     * @returns Whether this operation is valid or not.
     */
    bool isValid() const;

    /**
     * @returns Whether this operation has finished with an error or not.
     *
     * @note This function returns a meaningful result only when @ref isFinished is true.
     */
    bool isError() const;
    /**
     * @returns The error name of the occurred error, if any.
     *
     * @note If @ref isError returns false, this function will return an empty string.
     *
     * @note This function returns a meaningful result only when @ref isFinished is true.
     */
    QString errorName() const;
    /**
     * @returns The error message of the occurred error, if any.
     *
     * @note If @ref isError returns false, this function will return an empty string. Given that
     *       it is not compulsory to provide an error message for Operations, the returned string
     *       might as well be empty even in case the Operation failed.
     *
     * @note This function returns a meaningful result only when @ref isFinished is true.
     */
    QString errorMessage() const;

    /**
     * @brief Synchronously awaits for the Operation's completion.
     *
     * This method allows to process an Operation synchronously, awaiting for its
     * completion and returning when the Operation is finished. It returns whether the
     * operation finished successfully or not, whereas further information can be
     * retrieved from the Operation object once this method returns.
     *
     * It is possible to specify a timeout after which synchronize will return regardless
     * of the Operation's completion.
     *
     * @p timeout Timeout, in seconds, after which the synchronization will fail. A negative
     *            value implies the synchronization will never time out.
     *
     * @returns Whether the operation succeeded or not.
     *
     * @note If the operation has already finished, this method returns immediately. It
     *       returns the result of the operation nonetheless.
     *
     * @note If synchronize times out, the Operation won't fail or will be stopped. If you set a
     *       timeout and synchronize returns false, you should be checking @ref isFinished to
     *       make sure the Operation has failed or timed out synchronizing.
     *
     * @note As much as synchronize is used extensively in bindings and is safe to use,
     *       it is not part of the advised paradigm of development in Hemera Qt5, which
     *       strives to be as asynchronous as possible. If you are using Qt5 SDK directly,
     *       connecting to @ref finished is usually the advised choice.
     */
    bool synchronize(int timeout = -1);

Q_SIGNALS:
    /**
     * @brief Notifies the completion of the asynchronous operation.
     *
     * Emitted when the operation has finished. Connect to this signal to inspect the Operation result upon
     * its completion.
     *
     * @param operation A pointer to the finished operation.
     */
    void finished(Hemera::DeveloperMode::Operation *operation);

    void progress(int percentage);
    void downloadInfo(quint64 downloaded, quint64 total, quint64 rate);

protected:
    explicit Operation(QObject *parent = Q_NULLPTR);

protected Q_SLOTS:
    /**
     * Implements the operation main logic
     *
     * When implementing an Operation, this method should hold your main logic.
     * It is called by Operation when needed, and the developer should just reimplement it without invoking it.
     *
     * Once the Operation is completed, either setFinished or setFinishedWithError must be called.
     */
    virtual void startImpl() = 0;

    /**
     * Completes the processing of the Operation
     *
     * Once setFinished is called, the Operation will be declared successfully completed.
     *
     * @note After calling this function, the object will be garbage collected, and no further processing
     *       must be done.
     */
    void setFinished();
    /**
     * Completes the processing of the Operation with a failure
     *
     * Once setFinishedWithError is called, the Operation will be declared failed.
     *
     * @note After calling this function, the object will be garbage collected, and no further processing
     *       must be done.
     */
    void setFinishedWithError(const QString &name, const QString &message);

private:
    class Private;
    Private * const d;
};

}
}

#endif // HEMERA_DEVELOPERMODE_OPERATION_H
