#ifndef DEVICECOMMAND_H
#define DEVICECOMMAND_H

#include "basecommand.h"

class QTimer;

class DeviceCommand : public BaseCommand
{
    Q_OBJECT

public:
    DeviceCommand(QObject *parent = 0);
    virtual ~DeviceCommand();

    virtual QString name();
    virtual QString briefDescription();
    virtual QString longDescription();
};

class AddDeviceCommand : public BaseCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(AddDeviceCommand)

public:
    explicit AddDeviceCommand(QObject* parent = 0);
    virtual ~AddDeviceCommand();

    virtual QString briefDescription();
    virtual QString longDescription();
    virtual QString name();

protected:
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments);
    virtual void setupParser(QCommandLineParser* parser, const QStringList& arguments);
};

class RemoveDeviceCommand : public BaseCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(RemoveDeviceCommand)

public:
    explicit RemoveDeviceCommand(QObject* parent = 0);
    virtual ~RemoveDeviceCommand();

    virtual QString briefDescription();
    virtual QString longDescription();
    virtual QString name();

protected:
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments);
    virtual void setupParser(QCommandLineParser* parser, const QStringList& arguments);
};

class ScanDeviceCommand : public BaseCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(ScanDeviceCommand)

public:
    explicit ScanDeviceCommand(QObject* parent = 0);
    virtual ~ScanDeviceCommand();

    virtual QString briefDescription();
    virtual QString longDescription();
    virtual QString name();

protected:
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments);

private:
    QTimer *m_timeoutTimer;
};

class ListDeviceCommand : public BaseCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(ListDeviceCommand)

public:
    explicit ListDeviceCommand(QObject* parent = 0);
    virtual ~ListDeviceCommand();

    virtual QString briefDescription();
    virtual QString longDescription();
    virtual QString name();

protected:
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments);
};

class AssociateDeviceCommand : public BaseCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(AssociateDeviceCommand)

public:
    explicit AssociateDeviceCommand(QObject* parent = 0);
    virtual ~AssociateDeviceCommand();

    virtual QString briefDescription();
    virtual QString longDescription();
    virtual QString name();

protected:
    virtual bool parseAndExecute(QCommandLineParser* parser, const QStringList& arguments);
    virtual void setupParser(QCommandLineParser* parser, const QStringList& arguments);
};

#endif // DEVICECOMMAND_H
