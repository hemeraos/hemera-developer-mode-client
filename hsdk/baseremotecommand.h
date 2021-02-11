#ifndef BASEREMOTECOMMAND_H
#define BASEREMOTECOMMAND_H

#include "basecommand.h"

#include <QtCore/QCommandLineOption>
#include <hemeradevelopermodecontroller.h>

namespace Hemera {
namespace DeveloperMode {
class Controller;
class ShellOperation;
}
}

class StdInReader : public QObject
{
    Q_OBJECT
public:
    explicit StdInReader(QObject* parent = 0);
    virtual ~StdInReader();

public Q_SLOTS:
    void start();

Q_SIGNALS:
    void stdInData(const QByteArray &data);
    void stdInEOF();
};

class BaseRemoteCommand : public BaseCommand
{
    Q_OBJECT

public:
    BaseRemoteCommand(QObject *parent = 0);

    Hemera::DeveloperMode::ShellOperation *remoteExecute(const QString &command, const QStringList &arguments = QStringList(), const QString &workingDirectory = QString());

protected:
    virtual void setupParser(QCommandLineParser *parser, const QStringList &arguments);
    virtual bool parseAndExecute(QCommandLineParser *parser, const QStringList &arguments);

    QCommandLineOption m_emulatorOption;
    QCommandLineOption m_deviceOption;

Q_SIGNALS:
    void stdoutMessage(const QByteArray &message);

private:
    Hemera::DeveloperMode::Controller::Ptr m_controller;
};

#endif // BASEREMOTECOMMAND_H
