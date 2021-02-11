#ifndef VIRTUALBOXCOMMANDS_P_H
#define VIRTUALBOXCOMMANDS_P_H

#include <QtCore/QHash>
#include <QtCore/QStringList>

#include "hemeradevelopermodeexport.h"

namespace VirtualBox {

class HemeraDeveloperModeClient_EXPORT VirtualMachineInfo
{
public:
    VirtualMachineInfo();
    QString id;
    QHash< QString, QString > sharedDirs;
    QHash< QString, quint16 > forwardedPorts;
    QString mainDriveId;
    QStringList macs;
    bool headless;
};

HemeraDeveloperModeClient_EXPORT QString vBoxManagePath();
HemeraDeveloperModeClient_EXPORT VirtualMachineInfo virtualMachineInfo(const QString &vmName);
HemeraDeveloperModeClient_EXPORT QStringList runningVirtualMachines();
HemeraDeveloperModeClient_EXPORT bool isVirtualMachineRunning(const QString &vmName);
HemeraDeveloperModeClient_EXPORT QHash< QString, QString > availableVirtualMachines();

HemeraDeveloperModeClient_EXPORT bool startVirtualMachine(const QString &vmName, bool headless);
HemeraDeveloperModeClient_EXPORT bool killVirtualMachine(const QString &vmName);

HemeraDeveloperModeClient_EXPORT bool createVMForEmulator(const QString &name);
HemeraDeveloperModeClient_EXPORT bool deleteVM(const QString &query, bool deleteFiles);
HemeraDeveloperModeClient_EXPORT bool createStorageControllerForEmulator(const VirtualBox::VirtualMachineInfo &vminfo);
HemeraDeveloperModeClient_EXPORT bool attachLocalStorageToEmulator(const VirtualBox::VirtualMachineInfo &vminfo, const QString &pathToFile);
HemeraDeveloperModeClient_EXPORT bool detachLocalStorageFromEmulator(const VirtualBox::VirtualMachineInfo &vminfo, bool deleteDisk);
HemeraDeveloperModeClient_EXPORT bool setPortsForEmulator(const VirtualBox::VirtualMachineInfo &vminfo);
HemeraDeveloperModeClient_EXPORT bool setSharedFoldersForEmulator(const VirtualBox::VirtualMachineInfo &vminfo);

}

#endif
