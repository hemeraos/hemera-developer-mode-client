#ifndef HELPERMACROS_H
#define HELPERMACROS_H

#include "remotebuildcommand.h"

#define DECLARE_SIMPLE_REMOTE_BUILD_COMMAND(CLASSNAME, CMDNAME, BRIEF_DESCRIPTION, LONG_DESCRIPTION, REMOTE_BUILD_COMMAND) \
class CLASSNAME : public RemoteBuildCommand\
{\
    Q_OBJECT \
public: \
    CLASSNAME(QObject *parent = 0) : RemoteBuildCommand(parent) {} \
    virtual QString name() { return CMDNAME; } \
    virtual QString briefDescription() { return tr(BRIEF_DESCRIPTION); } \
    virtual QString longDescription() { return tr(LONG_DESCRIPTION); } \
protected: \
    virtual QString remoteBuildCommand() { return REMOTE_BUILD_COMMAND; } \
};

#define DECLARE_SIMPLE_REMOTE_COMMAND(CLASSNAME, CMDNAME, BRIEF_DESCRIPTION, LONG_DESCRIPTION, REMOTE_COMMAND, COMMAND_ARGUMENTS) \
class CLASSNAME : public BaseRemoteCommand \
{ \
    Q_OBJECT \
public: \
    CLASSNAME(QObject *parent = 0) : BaseRemoteCommand(parent) {} \
    virtual QString name() { return CMDNAME; } \
    virtual QString briefDescription() { return tr(BRIEF_DESCRIPTION); } \
    virtual QString longDescription() { return tr(LONG_DESCRIPTION); } \
protected: \
    virtual bool parseAndExecute(QCommandLineParser *parser, const QStringList &arguments) {\
        if (!BaseRemoteCommand::parseAndExecute(parser, arguments)) {\
            return false;\
        }\
        remoteExecute(REMOTE_COMMAND, COMMAND_ARGUMENTS);\
        return true;\
    }\
};

#define DECLARE_SIMPLE_REMOTE_COMMAND_POSITIONAL(CLASSNAME, CMDNAME, BRIEF_DESCRIPTION, LONG_DESCRIPTION, REMOTE_COMMAND) \
DECLARE_SIMPLE_REMOTE_COMMAND(CLASSNAME, CMDNAME, BRIEF_DESCRIPTION, LONG_DESCRIPTION, REMOTE_COMMAND, parser->positionalArguments())

#endif
