#ifndef OUTPUTCOMMAND_H
#define OUTPUTCOMMAND_H

#include "basecommand.h"

class OutputCommand : public BaseCommand
{
    Q_OBJECT

public:
    OutputCommand(QObject *parent = 0);

    virtual QString name();
    virtual QString briefDescription();
    virtual QString longDescription();

protected:
    virtual void setupParser(QCommandLineParser *parser, const QStringList &arguments);
    virtual bool parseAndExecute(QCommandLineParser *parser, const QStringList &arguments);

private:
    QCommandLineOption m_emulatorOption;
    QCommandLineOption m_deviceOption;
};

#endif // OUTPUTCOMMAND_H
