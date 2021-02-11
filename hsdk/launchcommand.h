/*
 *
 */

#ifndef LAUNCHCOMMAND_H
#define LAUNCHCOMMAND_H

#include "basecommand.h"

#include <hemeradevelopermodecontroller.h>

namespace Hemera {
namespace DeveloperMode {
class ApplicationOutput;
class Controller;
class Star;
class Target;
}
}

class QTimer;
class SshHandler;
class LaunchCommand : public BaseCommand
{
    Q_OBJECT

public:
    explicit LaunchCommand(QObject* parent = 0);
    virtual ~LaunchCommand();

    virtual QString longDescription() Q_DECL_OVERRIDE;
    virtual QString briefDescription() Q_DECL_OVERRIDE;
    virtual QString name() Q_DECL_OVERRIDE;

protected:
    virtual void setupParser(QCommandLineParser* parser, const QStringList& arguments) Q_DECL_OVERRIDE;
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments) Q_DECL_OVERRIDE;
    virtual bool onTermRequest() Q_DECL_OVERRIDE;

private:
    QCommandLineOption m_emulatorOption;
    QCommandLineOption m_deviceOption;
    QCommandLineOption m_debugOption;
    QCommandLineOption m_starOption;

    QTimer *m_timeoutTimer;

    Hemera::DeveloperMode::ApplicationOutput *m_applicationOutput;
    Hemera::DeveloperMode::Controller::Ptr m_controller;
    Hemera::DeveloperMode::Target::Ptr m_target;

    bool m_shuttingDown;
    bool m_isRunning;

    QString m_debugMode;
    Hemera::DeveloperMode::Star *m_star;
    QString m_applicationId;
};

#endif // LAUNCHCOMMAND_H
