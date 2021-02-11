#ifndef HACOMMAND_H
#define HACOMMAND_H

#include "basecommand.h"

class HaCommand : public BaseCommand
{
    Q_OBJECT

public:
    HaCommand(QObject *parent = 0);
    virtual ~HaCommand();

    virtual QString name();
    virtual QString briefDescription();
    virtual QString longDescription();
};

class InspectHaCommand : public BaseCommand
{
    Q_OBJECT

public:
    InspectHaCommand(QObject *parent = 0);
    virtual ~InspectHaCommand();

    virtual QString name();
    virtual QString briefDescription();
    virtual QString longDescription();

protected:
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments) override;
    virtual void setupParser(QCommandLineParser* parser, const QStringList& arguments) override;
};

class AddFileToHaCommand : public BaseCommand
{
    Q_OBJECT

public:
    AddFileToHaCommand(QObject *parent = 0);
    virtual ~AddFileToHaCommand();

    virtual QString name();
    virtual QString briefDescription();
    virtual QString longDescription();

protected:
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments) override;
    virtual void setupParser(QCommandLineParser* parser, const QStringList& arguments) override;
};

#endif // HACOMMAND_H
