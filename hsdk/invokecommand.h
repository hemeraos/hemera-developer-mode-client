#ifndef INVOKECOMMAND_H
#define INVOKECOMMAND_H

#include <remotebuildcommand.h>

class InvokeCommand : public RemoteBuildCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(InvokeCommand)

public:
    explicit InvokeCommand(QObject* parent = 0);
    virtual ~InvokeCommand();

    virtual QString longDescription() override;
    virtual QString briefDescription() override;
    virtual QString name() override;

protected:
    virtual QString remoteBuildCommand() override;
    virtual void setupParser(QCommandLineParser* parser, const QStringList& arguments) override;
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments) override;

private:
    QString m_cachedOutput;

    QCommandLineOption m_failOnOffline;
};

#endif // INVOKECOMMAND_H
