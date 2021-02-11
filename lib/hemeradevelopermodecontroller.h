/*
 *
 */

#ifndef HEMERA_DEVELOPERMODE_CONTROLLER_H
#define HEMERA_DEVELOPERMODE_CONTROLLER_H

#include "hemeradevelopermodeexport.h"

#include "hemeradevelopermodetarget.h"

#include <QtCore/QObject>

#include <QtCore/QDir>
#include <QtCore/QFlags>
#include <QtCore/QSharedPointer>

class QIODevice;

namespace Hemera {
namespace DeveloperMode {

class HyperspaceStream;

class DeployOperation;
class ShellOperation;

class Target;
class TargetPrivate;

class HemeraDeveloperModeClient_EXPORT Controller : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Controller)

    Q_PRIVATE_SLOT(d, void dataFromDevice())

public:
    typedef QSharedPointer<Controller> Ptr;
    typedef QSharedPointer<const Controller> ConstPtr;

    enum class Status : quint8 {
        Invalid = 0,
        Stopped,
        Running,
        Switching
    };
    Q_ENUMS(Status)

    enum Feature {
        NoFeatures = 0,
        Audio = 1 << 0,
        Video = 1 << 1,
        SerialPorts = 1 << 2,
        Console = 1 << 3,
        Printers = 1 << 4,
        Disks = 1 << 5,
        Hyperspace = 1 << 6,
        Network = 1 << 7,
        /// Software management
        CheckForUpdates = 1 << 10,
        DownloadUpdates = 1 << 11,
        UpdateApplications = 1 << 12,
        InstallApplications = 1 << 13,
        RemoveApplications = 1 << 14,
        ManageSoftwareRepositories = 1 << 15,
        UpdateSystem = 1 << 16,
        /// Other features
        SoftwareKeyboard = 1 << 20,
        DoNotRestartOnUpdate = 1 << 21,
        /// This enables groups such as cdrom, floppy and tape
        LegacyDevices = 1 << 31
    };
    Q_DECLARE_FLAGS(Features, Feature)
    Q_FLAGS(Features)
    Q_ENUMS(Feature)

    virtual ~Controller();

    /// Once invalidated, it has to be discarded.
    bool isValid() const;

    QHash< QString, Status > statuses() const;
    Status statusOf(const QString &star) const;

    void startSimple(const QString &star, const QString &application, bool withDebug = false,
                     const QString &debugPrefix = QString(), const QString &debugSuffix = QString());

    void startAdvanced(const QString &star, const QString &applications, Features features, bool withDebug = false,
                       const QString &debugPrefix = QString(), const QString &debugSuffix = QString());

    void stop(const QString &star);

    static QString localPathToVm(const QDir &localDirectory = QDir::current());

    ShellOperation *executeShellCommand(const QString &program, const QStringList &arguments,
                                        const QStringList &environment = QStringList(), const QString &workingDirectory = localPathToVm());

public Q_SLOTS:
    DeployOperation *deployPackage(const QString &filePath);

    void release();

Q_SIGNALS:
    void statusChanged(const QString &star, Hemera::DeveloperMode::Controller::Status status);
    void error(const QString &errorName, const QString &errorMessage);
    void disconnected();

private:
    explicit Controller(const Target::Ptr &parent, HyperspaceStream *device);

    // This method is here so that users might hack into the protocol, if needed. It might be even protected one day...
    QString sendRequest(const QString& command, const QString& star, const QJsonObject& data);

    class Private;
    Private * const d;

    friend class ShellOperation;
    friend class Target;
    friend class TargetPrivate;
};

}
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Hemera::DeveloperMode::Controller::Features)

Q_DECLARE_METATYPE(Hemera::DeveloperMode::Controller::Features)
Q_DECLARE_METATYPE(Hemera::DeveloperMode::Controller::Status)

#endif // HEMERA_DEVELOPERMODE_CONTROLLER_H
