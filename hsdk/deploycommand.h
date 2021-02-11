#ifndef DEPLOYCOMMAND_H
#define DEPLOYCOMMAND_H

#include "basecommand.h"

#include <hemeradevelopermodecontroller.h>

class QTimer;
namespace Hemera {
namespace DeveloperMode {
class Target;
class Controller;
}
}

class ScpHandler;
class SshHandler;

class DeployCommand : public BaseCommand
{
    Q_OBJECT

public:
    DeployCommand(QObject *parent = 0);

    virtual QString name();
    virtual QString briefDescription();
    virtual QString longDescription();

protected:
    virtual void setupParser(QCommandLineParser *parser, const QStringList &arguments);
    virtual bool parseAndExecute(QCommandLineParser *parser, const QStringList &arguments);

private:
    QCommandLineOption m_emulatorOption;
    QCommandLineOption m_deviceOption;
    Hemera::DeveloperMode::Controller::Ptr m_controller;
    QString m_rpmName;
    QTimer *m_timeoutTimer;
};

#endif // DEPLOYCOMMAND_H
