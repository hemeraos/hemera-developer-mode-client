#ifndef EMULATORCOMMAND_H
#define EMULATORCOMMAND_H

#include "basecommand.h"

class EmulatorCommand : public BaseCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(EmulatorCommand)

public:
    EmulatorCommand(QObject *parent = 0);
    virtual ~EmulatorCommand();

    virtual QString longDescription();
    virtual QString briefDescription();
    virtual QString name();

private:
};

class InstallEmulatorCommand : public BaseCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(InstallEmulatorCommand)

public:
    explicit InstallEmulatorCommand(QObject* parent = 0);
    virtual ~InstallEmulatorCommand();

    virtual QString briefDescription();
    virtual QString longDescription();
    virtual QString name();

protected:
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments);
    virtual void setupParser(QCommandLineParser* parser, const QStringList& arguments);

private:
    QCommandLineOption m_keepOption;
    QCommandLineOption m_moveVDIOption;
    QCommandLineOption m_startServerOption;
};

class RemoveEmulatorCommand : public BaseCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(RemoveEmulatorCommand)

public:
    explicit RemoveEmulatorCommand(QObject* parent = 0);
    virtual ~RemoveEmulatorCommand();

    virtual QString briefDescription();
    virtual QString longDescription();
    virtual QString name();

protected:
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments);
    virtual void setupParser(QCommandLineParser* parser, const QStringList& arguments);

private:
    QCommandLineOption m_keepOption;
};

class ListEmulatorsCommand : public BaseCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(ListEmulatorsCommand)

public:
    explicit ListEmulatorsCommand(QObject* parent = 0);
    virtual ~ListEmulatorsCommand();

    virtual QString briefDescription();
    virtual QString longDescription();
    virtual QString name();

protected:
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments);
};

class ListVMCommand : public BaseCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(ListVMCommand)

public:
    explicit ListVMCommand(QObject* parent = 0);
    virtual ~ListVMCommand();

    virtual QString briefDescription();
    virtual QString longDescription();
    virtual QString name();

protected:
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments);
};

class StartEmulatorCommand : public BaseCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(StartEmulatorCommand)

public:
    explicit StartEmulatorCommand(QObject* parent = 0);
    virtual ~StartEmulatorCommand();

    virtual QString briefDescription();
    virtual QString longDescription();
    virtual QString name();

protected:
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments);
    virtual void setupParser(QCommandLineParser* parser, const QStringList& arguments);

private:
    QCommandLineOption m_headlessOption;
};

class StopEmulatorCommand : public BaseCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(StopEmulatorCommand)

public:
    explicit StopEmulatorCommand(QObject* parent = 0);
    virtual ~StopEmulatorCommand();

    virtual QString briefDescription();
    virtual QString longDescription();
    virtual QString name();

protected:
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments);
    virtual void setupParser(QCommandLineParser* parser, const QStringList& arguments);
};

#endif // EMULATORCOMMAND_H
