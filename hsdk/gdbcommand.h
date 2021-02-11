/*
 *
 */

#ifndef GDBCOMMAND_H
#define GDBCOMMAND_H

#include "baseremotecommand.h"

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
class GDBCommand : public BaseRemoteCommand
{
    Q_OBJECT

public:
    explicit GDBCommand(QObject* parent = 0);
    virtual ~GDBCommand();

    virtual QString longDescription() Q_DECL_OVERRIDE;
    virtual QString briefDescription() Q_DECL_OVERRIDE;
    virtual QString name() Q_DECL_OVERRIDE;

protected:
    virtual void setupParser(QCommandLineParser* parser, const QStringList& arguments) Q_DECL_OVERRIDE;
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments) Q_DECL_OVERRIDE;
    virtual bool onTermRequest() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void term();

private:
    QCommandLineOption m_emulatorOption;
    QCommandLineOption m_deviceOption;
    QCommandLineOption m_interfaceOption;

    QTimer *m_timeoutTimer;

    Hemera::DeveloperMode::ApplicationOutput *m_applicationOutput;

    bool m_shuttingDown;
    bool m_isRunning;

    QString m_debugMode;
    Hemera::DeveloperMode::Star *m_star;
    QString m_applicationId;
};

#endif // GDBCOMMAND_H
