#ifndef REMOTEBUILDCOMMAND_H
#define REMOTEBUILDCOMMAND_H

#include "baseremotecommand.h"
#include <QCommandLineOption>

class RemoteBuildCommand : public BaseRemoteCommand
{
    Q_OBJECT

public:
    RemoteBuildCommand(QObject *parent = 0);

protected:
    virtual void setupParser(QCommandLineParser *parser, const QStringList &arguments);
    virtual bool parseAndExecute(QCommandLineParser *parser, const QStringList &arguments);

    virtual QString remoteBuildCommand() = 0;
    virtual QString extraArguments(QCommandLineParser *parser, const QStringList &arguments);
};

#endif // REMOTEBUILDCOMMAND_H
