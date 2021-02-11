#ifndef BASECOMMAND_H
#define BASECOMMAND_H

#include <QtCore/QCommandLineParser>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QPointer>

#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodetarget.h>

namespace Hemera {
namespace DeveloperMode {
class Device;
class Emulator;
class Target;
class Star;
class Controller;
}
}

class BaseCommand : public QObject
{
    Q_OBJECT

public:
    explicit BaseCommand(QObject *parent = 0);

    virtual QString name() = 0;
    virtual QString briefDescription() = 0;
    virtual QString longDescription() = 0;

    void execute(const QStringList &arguments, int depth = 1);

    bool          hasSubcommands();
    QStringList   subcommands();
    bool          isSubcommand(const QString &name);
    BaseCommand * subcommand(const QString &name);
    void          addSubcommand(BaseCommand *command);
    BaseCommand * removeSubcommand(const QString &name);

    enum SignalHandlingBehavior {
        DefaultSignalHandlingBehavior = 0,
        AllowMultipleSIGINT = 1
    };
    Q_DECLARE_FLAGS(SignalHandlingBehaviors, SignalHandlingBehavior)


    SignalHandlingBehaviors signalHandlingBehaviors() const;
    /*
     * Command and subcommands signal handling combined behavior
     */
    void setSignalHandlingBehaviors(SignalHandlingBehaviors);
    // Take any action on SIGTERM (or WIN32 equivalent). Don't reimplement if you don't know what you are doing.
    // Return true if it's being handled.
    virtual bool onTermRequest();

protected:
    // a real command can extend this method to add common options
    // or positional arguments to the parser
    virtual void setupParser(QCommandLineParser *parser, const QStringList &arguments);
    // a real command must extend this method to parse its common options
    // and positional arguments and perform its operation
    virtual bool parseAndExecute(QCommandLineParser *parser, const QStringList &arguments);

    void printErrorAndExit(const QString &errorMessage, int exitStatus = 1);

    Hemera::DeveloperMode::Target::Ptr retrieveTargetOrDie(const QString &targetName, bool connected = true);
    Hemera::DeveloperMode::Device::Ptr retrieveDeviceOrDie(const QString &deviceName, bool connected = true);
    Hemera::DeveloperMode::Emulator::Ptr retrieveAnyEmulatorOrDie(bool connected = true);
    Hemera::DeveloperMode::Emulator::Ptr retrieveEmulatorOrDie(const QString &emulatorName, bool connected = true);
    Hemera::DeveloperMode::Emulator::Ptr retrieveEmulatorForDeviceOrDie(const QString &deviceName, bool connected = true);
    Hemera::DeveloperMode::Star *retrieveStarOrDie(Hemera::DeveloperMode::Target::Ptr target, const QString &star);
    Hemera::DeveloperMode::Star *retrieveStarOrDie(const QString &targetName, const QString &star);
    QSharedPointer<Hemera::DeveloperMode::Controller> retrieveDeveloperModeControllerOrDie(Hemera::DeveloperMode::Target::Ptr target);
    QSharedPointer<Hemera::DeveloperMode::Controller> retrieveDeveloperModeControllerOrDie(const QString &targetName);

    Hemera::DeveloperMode::Target::Ptr loadTarget(QCommandLineParser *parser, QCommandLineOption emulatorOption, QCommandLineOption deviceOption,
                                                  bool connected = true, bool retrieveDefaultTarget = false);
    Hemera::DeveloperMode::Emulator::Ptr loadEmulatorEnvironment(QCommandLineParser *parser, QCommandLineOption emulatorOption,
                                                                 QCommandLineOption deviceOption, bool connected = true);

private:
    QMap<QString, BaseCommand*> m_subcommands;
    int m_maxCommandSize;
    QPointer< BaseCommand > m_runningSubcommand;
    SignalHandlingBehaviors m_signalHandlingBehaviors;

Q_SIGNALS:
    void finished(int exitStatus);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(BaseCommand::SignalHandlingBehaviors)

#endif // BASECOMMAND_H
