#include "virtualboxcommands_p.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QThreadPool>

namespace VirtualBox {

// Define statically control strings for VBoxManage
const char VBOXMANAGE[] = "VBoxManage";
const char LIST[] = "list";
const char RUNNINGVMS[] = "runningvms";
const char CREATEVM[] = "createvm";
const char MODIFYVM[] = "modifyvm";
const char STORAGEATTACH[] = "storageattach";
const char STORAGECTL[] = "storagectl";
const char CLOSEMEDIUM[] = "closemedium";
const char UNREGISTERVM[] = "unregistervm";
const char DISK[] = "disk";
const char LINUX[] = "Linux";
const char NAT[] = "nat";
const char VIRTIO[] = "virtio";
const char STORAGE_CTL_OPT[] = "--storagectl";
const char ADD_STORAGE_CTL_OPT[] = "--add";
const char NAME_STORAGE_CTL_OPT[] = "--name";
const char PORTCOUNT_OPT[] = "--portcount";
const char BOOTABLE_OPT[] = "--bootable";
const char PORT_OPT[] = "--port";
const char MEDIUM_OPT[] = "--medium";
const char NAME_OPT[] = "--name";
const char TYPE_OPT[] = "--type";
const char DELETE_OPT[] = "--delete";
const char GROUPS_OPT[] = "--groups";
const char OSTYPE_OPT[] = "--ostype";
const char CPUS_OPT[] = "--cpus";
const char REGISTER_OPT[] = "--register";
const char MEMORY_OPT[] = "--memory";
const char VRAM_OPT[] = "--vram";
const char IOAPIC_OPT[] = "--ioapic";
const char AUDIOCONTROLLER_OPT[] = "--audiocontroller";
const char NIC1_OPT[] = "--nic1";
const char NICTYPE1_OPT[] = "--nictype1";
const char AUDIO_OPT[] = "--audio";
const char VMS[] = "vms";
const char SHOWVMINFO[] = "showvminfo";
const char MACHINE_READABLE[] = "--machinereadable";
const char STARTVM[] = "startvm";
const char CONTROLVM[] = "controlvm";
const char NATPF1[] = "--natpf1";
const char ACPI_POWER_BUTTON[] = "acpipowerbutton";
const char POWEROFF[] = "poweroff";
const char TYPE[] = "--type";
const char HEADLESS[] = "headless";
const char SHAREDFOLDER[] = "sharedfolder";
const char SHARE_NAME[] = "--name";
const char REMOVE_SHARED[] = "remove";
const char HOSTPATH[] = "--hostpath";
const char ADD_SHARED[] = "add";

static VirtualMachineInfo virtualMachineInfoFromOutput(const QString &output);
static bool isVirtualMachineListed(const QString &vmName, const QString &output);
static bool isVirtualMachineRegistered(const QString &vmName);

VirtualMachineInfo::VirtualMachineInfo()
    : headless(false)
{
}

QString vBoxManagePath() {
#ifdef Q_OS_WIN
        static QString path;
        if (path.isEmpty()) {
            path = QString::fromLocal8Bit(qgetenv("VBOX_INSTALL_PATH"));
            if (path.isEmpty()) {
                // env var name for VirtualBox 4.3.12 changed to this
                path = QString::fromLocal8Bit(qgetenv("VBOX_MSI_INSTALL_PATH"));
                if (path.isEmpty()) {
                    // Not found in environment? Look up registry.
                    QSettings s(QLatin1String("HKEY_LOCAL_MACHINE\\SOFTWARE\\Oracle\\VirtualBox"),
                                QSettings::NativeFormat);
                    path = s.value(QLatin1String("InstallDir")).toString();
                    if (path.startsWith(QLatin1Char('"')) && path.endsWith(QLatin1Char('"')))
                        path = path.mid(1, path.length() - 2); // remove quotes
                }
            }

            if (!path.isEmpty())
                path.append(QDir::separator() + QLatin1String(VBOXMANAGE));
        }
        return path;
#else
        return QLatin1String(VBOXMANAGE);
#endif

}

VirtualMachineInfo virtualMachineInfo(const QString &vmName)
{
    VirtualMachineInfo info;
    QStringList arguments;
    arguments.append(QLatin1String(SHOWVMINFO));
    arguments.append(vmName);
    arguments.append(QLatin1String(MACHINE_READABLE));
    QProcess process;
    process.start(vBoxManagePath(), arguments);
    if (!process.waitForFinished())
        return info;

    return virtualMachineInfoFromOutput(QString::fromLocal8Bit(process.readAllStandardOutput()));
}

VirtualMachineInfo virtualMachineInfoFromOutput(const QString &output)
{
    VirtualMachineInfo info;

    // Get ssh port, shared home and shared targets
    // 1 Name, 2 Protocol, 3 Host IP, 4 Host Port, 5 Guest IP, 6 Guest Port, 7 Shared Folder Name,
    // 8 Shared Folder Path 9 mac
    QRegExp rexp(QLatin1String("(?:Forwarding\\(\\d+\\)=\"(\\w+),(\\w+),(.*),(\\d+),(.*),(\\d+)\")"
                               "|(?:SharedFolderNameMachineMapping\\d+=\"(\\w+)\"\\W*"
                               "SharedFolderPathMachineMapping\\d+=\"(.*)\")"
                               "|(?:macaddress\\d+=\"(.*)\")"
                               "|(?:hardwareuuid=\"(.*)\")"
                               "|(?:\"hemeradisk-ImageUUID-0-0\"=\"(.*)\")"
                               "|(?:SessionType=\"(.*)\")"));

    rexp.setMinimal(true);
    int pos = 0;
    while ((pos = rexp.indexIn(output, pos)) != -1) {
        pos += rexp.matchedLength();
        if (rexp.cap(0).startsWith(QLatin1String("Forwarding"))) {
            quint16 port = rexp.cap(4).toUInt();
            info.forwardedPorts.insert(rexp.cap(1), port);
        } else if(rexp.cap(0).startsWith(QLatin1String("SharedFolderNameMachineMapping"))) {
            info.sharedDirs.insert(rexp.cap(7), rexp.cap(8));
        } else if(rexp.cap(0).startsWith(QLatin1String("macaddress"))) {
            QRegExp rx(QLatin1String("(?:([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2}))"));
            QString mac = rexp.cap(9);
            QStringList macFields;
            if(rx.exactMatch(mac)) {
                macFields = rx.capturedTexts();
            }
            if(!macFields.isEmpty()) {
                macFields.removeFirst();
                info.macs << macFields.join(QLatin1String(":"));
            }
        } else if (rexp.cap(0).startsWith(QLatin1String("SessionType"))) {
            info.headless = rexp.cap(10) == QLatin1String("headless");
            // FIXME: I hate regexps so much...
        } else if (rexp.cap(0).startsWith(QStringLiteral("hardwareuuid"))) {
            info.id = rexp.cap(0).split(QLatin1Char('"')).at(1);
        } else if (rexp.cap(0).startsWith(QStringLiteral("\"hemeradisk-ImageUUID-0-0\""))) {
            info.mainDriveId = rexp.cap(0).split(QLatin1Char('"')).at(1);
        }
    }

    return info;
}

QStringList runningVirtualMachines()
{
    QStringList arguments;
    arguments.append(QLatin1String(LIST));
    arguments.append(QLatin1String(RUNNINGVMS));
    QProcess process;
    process.start(vBoxManagePath(), arguments);
    if (!process.waitForFinished()) {
        return QStringList();
    }

    QStringList vms;
    while (process.canReadLine()) {
        QList< QByteArray > data = process.readLine().split(' ');
        if (data.size() <= 0) {
            continue;
        }
        QString vm = QString::fromLatin1(data.last());
        vm = vm.mid(vm.indexOf(QLatin1Char('{')) + 1, vm.indexOf(QLatin1Char('}')) - vm.indexOf(QLatin1Char('{')) - 1);
        vms.append(vm);
    }

    return vms;
}

QHash< QString, QString > availableVirtualMachines()
{
    QStringList vms;
    QHash< QString, QString > availableVms;
    QStringList arguments;
    arguments.append(QLatin1String(VirtualBox::LIST));
    arguments.append(QLatin1String(VirtualBox::VMS));
    QProcess process;
    process.start(VirtualBox::vBoxManagePath(), arguments);
    if (!process.waitForFinished())
        return availableVms;

    while (process.canReadLine()) {
        QByteArray line = process.readLine();
        QList< QByteArray > names = line.split(' ');
        QString name = QString::fromLatin1(names.first());
        QString id = QString::fromLatin1(names.last());
        id = id.mid(id.indexOf(QLatin1Char('{')) + 1, id.indexOf(QLatin1Char('}')) - id.indexOf(QLatin1Char('{')) - 1);
        name.chop(1);
        name.remove(0, 1);
        availableVms.insert(name, id);
    }

    return availableVms;
}

bool isVirtualMachineRunning(const QString &vmName)
{
    QStringList arguments;
    arguments.append(QLatin1String(LIST));
    arguments.append(QLatin1String(RUNNINGVMS));
    QProcess process;
    process.start(vBoxManagePath(), arguments);
    if (!process.waitForFinished())
        return false;

    return isVirtualMachineListed(vmName, QString::fromLocal8Bit(process.readAllStandardOutput()));
}

bool isVirtualMachineRegistered(const QString &vmName)
{
    QStringList arguments;
    arguments.append(QLatin1String(LIST));
    arguments.append(QLatin1String(VMS));
    QProcess process;
    process.start(vBoxManagePath(), arguments);
    if (!process.waitForFinished())
        return false;

    return isVirtualMachineListed(vmName, QString::fromLocal8Bit(process.readAllStandardOutput()));
}

bool isVirtualMachineListed(const QString &vmName, const QString &output)
{
    return output.contains(vmName);
}


/////////////// VBox Generic VM Management operations
bool startVirtualMachine(const QString &vmName, bool headless)
{
    // Start VM if it is not running
    if (isVirtualMachineRunning(vmName)) {
        return true;
    }

    if (!isVirtualMachineRegistered(vmName)) {
        qWarning() << "VirtualBox: VM not registered:" << vmName;
        return false;
    }

    QStringList arguments;
    arguments.append(QLatin1String(STARTVM));
    arguments.append(vmName);
    if (headless) {
        arguments.append(QLatin1String(TYPE));
        arguments.append(QLatin1String(HEADLESS));
    }

    QProcess process;
    process.start(VirtualBox::vBoxManagePath(), arguments);
    bool success = process.waitForFinished();

    return success && process.exitCode() == 0;
}

bool killVirtualMachine(const QString &vmName)
{
    if (!isVirtualMachineRunning(vmName)) {
        return true;
    }

    QStringList arguments;
    arguments.append(QLatin1String(CONTROLVM));
    arguments.append(vmName);
    arguments.append(QLatin1String(POWEROFF));

    QProcess process;
    process.start(VirtualBox::vBoxManagePath(), arguments);
    bool success = process.waitForFinished();

    return success && process.exitCode() == 0;
}


/////////////// VBox+Emulator operations
bool createVMForEmulator(const QString &name)
{
    // Create and register actual VM
    {
        QStringList arguments;
        arguments.append(QLatin1String(VirtualBox::CREATEVM));
        arguments.append(QLatin1String(VirtualBox::NAME_OPT));
        arguments.append(name);
        arguments.append(QLatin1String(VirtualBox::GROUPS_OPT));
        arguments.append(QStringLiteral("/Hemera/Emulators/Generated"));
        arguments.append(QLatin1String(VirtualBox::OSTYPE_OPT));
        arguments.append(QLatin1String(VirtualBox::LINUX));
        arguments.append(QLatin1String(VirtualBox::REGISTER_OPT));

        QProcess process;
        process.start(VirtualBox::vBoxManagePath(), arguments);
        bool success = process.waitForFinished();

        if (!success || process.exitCode() != 0) {
            return false;
        }
    }

    // Now, let's retrieve it
    VirtualBox::VirtualMachineInfo vminfo = VirtualBox::virtualMachineInfo(name);
    if (vminfo.id.isEmpty()) {
        return false;
    }

    // Good! Let's set some default values now.
    {
        QStringList arguments;
        arguments.append(QLatin1String(VirtualBox::MODIFYVM));
        arguments.append(name);
        arguments.append(QLatin1String(VirtualBox::MEMORY_OPT));
        arguments.append(QStringLiteral("512"));
        arguments.append(QLatin1String(VirtualBox::VRAM_OPT));
        arguments.append(QStringLiteral("64"));
        arguments.append(QLatin1String(VirtualBox::IOAPIC_OPT));
        arguments.append(QStringLiteral("on"));
        arguments.append(QLatin1String(VirtualBox::AUDIOCONTROLLER_OPT));
        arguments.append(QStringLiteral("ac97"));
#ifdef Q_OS_LINUX
        arguments.append(QLatin1String(VirtualBox::AUDIO_OPT));
        arguments.append(QStringLiteral("pulse"));
#endif
        arguments.append(QLatin1String(VirtualBox::NIC1_OPT));
        arguments.append(QLatin1String(VirtualBox::NAT));
        arguments.append(QLatin1String(VirtualBox::NICTYPE1_OPT));
        arguments.append(QLatin1String(VirtualBox::VIRTIO));

        // Multiple CPUs? Let's trust Qt.
        int ncpu = QThreadPool::globalInstance()->maxThreadCount();
        if (ncpu >= 4) {
            arguments.append(QLatin1String(VirtualBox::CPUS_OPT));
            arguments.append(QString::number(ncpu / 2));
        }

        QProcess process;
        process.start(VirtualBox::vBoxManagePath(), arguments);
        bool success = process.waitForFinished();

        if (!success || process.exitCode() != 0) {
            return false;
        }
    }

    return true;
}


bool deleteVM(const QString &query, bool deleteFiles)
{
    if (!isVirtualMachineRegistered(query)) {
        return false;
    }

    QStringList arguments;
    arguments.append(QLatin1String(VirtualBox::UNREGISTERVM));
    arguments.append(query);
    if (deleteFiles) {
        arguments.append(QLatin1String(VirtualBox::DELETE_OPT));
    }

    QProcess process;
    process.start(VirtualBox::vBoxManagePath(), arguments);
    bool success = process.waitForFinished();

    return success && process.exitCode() == 0;
}


bool createStorageControllerForEmulator(const VirtualBox::VirtualMachineInfo& vminfo)
{
    QStringList arguments;
    arguments.append(QLatin1String(VirtualBox::STORAGECTL));
    arguments.append(vminfo.id);
    arguments.append(QLatin1String(VirtualBox::NAME_STORAGE_CTL_OPT));
    arguments.append(QStringLiteral("hemeradisk"));
    arguments.append(QLatin1String(VirtualBox::ADD_STORAGE_CTL_OPT));
    arguments.append(QStringLiteral("sata"));
    arguments.append(QLatin1String(VirtualBox::PORTCOUNT_OPT));
    arguments.append(QStringLiteral("1"));
    arguments.append(QLatin1String(VirtualBox::BOOTABLE_OPT));
    arguments.append(QStringLiteral("on"));

    QProcess process;
    process.start(VirtualBox::vBoxManagePath(), arguments);
    bool success = process.waitForFinished();

    return success && process.exitCode() == 0;
}

bool attachLocalStorageToEmulator(const VirtualBox::VirtualMachineInfo& vminfo, const QString &pathToFile)
{
    QStringList arguments;
    arguments.append(QLatin1String(VirtualBox::STORAGEATTACH));
    arguments.append(vminfo.id);
    arguments.append(QLatin1String(VirtualBox::STORAGE_CTL_OPT));
    arguments.append(QStringLiteral("hemeradisk"));
    arguments.append(QLatin1String(VirtualBox::TYPE_OPT));
    arguments.append(QStringLiteral("hdd"));
    arguments.append(QLatin1String(VirtualBox::PORT_OPT));
    arguments.append(QStringLiteral("0"));
    arguments.append(QLatin1String(VirtualBox::MEDIUM_OPT));
    arguments.append(pathToFile);

    QProcess process;
    process.start(VirtualBox::vBoxManagePath(), arguments);
    bool success = process.waitForFinished();

    return success && process.exitCode() == 0;
}

bool detachLocalStorageFromEmulator(const VirtualBox::VirtualMachineInfo& vminfo, bool deleteDisk)
{
    // Detach
    {
        QStringList arguments;
        arguments.append(QLatin1String(VirtualBox::STORAGEATTACH));
        arguments.append(vminfo.id);
        arguments.append(QLatin1String(VirtualBox::STORAGE_CTL_OPT));
        arguments.append(QStringLiteral("hemeradisk"));
        arguments.append(QLatin1String(VirtualBox::TYPE_OPT));
        arguments.append(QStringLiteral("hdd"));
        arguments.append(QLatin1String(VirtualBox::PORT_OPT));
        arguments.append(QStringLiteral("0"));
        arguments.append(QLatin1String(VirtualBox::MEDIUM_OPT));
        arguments.append(QStringLiteral("none"));

        QProcess process;
        process.start(VirtualBox::vBoxManagePath(), arguments);
        bool success = process.waitForFinished();

        if (!success || process.exitCode() != 0) {
            return false;
        }
    }
    // Close medium
    {
        QStringList arguments;
        arguments.append(QLatin1String(VirtualBox::CLOSEMEDIUM));
        arguments.append(QLatin1String(VirtualBox::DISK));
        arguments.append(vminfo.mainDriveId);
        if (deleteDisk) {
            arguments.append(QLatin1String(VirtualBox::DELETE_OPT));
        }

        QProcess process;
        process.start(VirtualBox::vBoxManagePath(), arguments);
        // Don't fail on medium close
        bool success = process.waitForFinished();

        if (!success) {
            return false;
        }
    }

    return true;
}

bool setPortsForEmulator(const VirtualBox::VirtualMachineInfo& vminfo)
{
    // Check port forwarding.
    QList< quint16 > neededPorts = QList< quint16 >() << 8080 << 8081 << 6391 << 6392 << 2223;
    QHash< quint16, QString > namesForPorts;
    namesForPorts.insert(8080, QStringLiteral("hyperspacehttps"));
    namesForPorts.insert(8081, QStringLiteral("hyperspacehttp"));
    namesForPorts.insert(6391, QStringLiteral("hyperstreamhttps"));
    namesForPorts.insert(6392, QStringLiteral("hyperstreamhttp"));
    namesForPorts.insert(2223, QStringLiteral("ssh"));
    for (QHash< QString, quint16 >::const_iterator i = vminfo.forwardedPorts.constBegin(); i != vminfo.forwardedPorts.constEnd(); ++i) {
        neededPorts.removeOne(i.value());
    }

    QString formatTemplate = QStringLiteral("%1,tcp,127.0.0.1,%2,,%3");

    for (quint16 port : neededPorts) {
        // Add port forwarding for missing port
        QStringList arguments;
        arguments.append(QLatin1String(VirtualBox::MODIFYVM));
        arguments.append(vminfo.id);
        arguments.append(QLatin1String(VirtualBox::NATPF1));
        arguments.append(formatTemplate.arg(namesForPorts.value(port), QString::number(port), port == 2223 ? QStringLiteral("22") : QString::number(port)));

        QProcess process;
        process.start(VirtualBox::vBoxManagePath(), arguments);
        bool success = process.waitForFinished();

        if (!success || process.exitCode() != 0) {
            return false;
        }
    }

    return true;
}

bool setSharedFoldersForEmulator(const VirtualBox::VirtualMachineInfo& vminfo)
{
    // Check shared directory
    bool hasSharedHome = false;
    for (const QString &value : vminfo.sharedDirs) {
        if (QDir(value).path() == QDir::homePath()) {
            hasSharedHome = true;
            break;
        }
    }

    if (!hasSharedHome) {
        // Create it.
        QStringList arguments;
        arguments.append(QLatin1String(VirtualBox::SHAREDFOLDER));
        arguments.append(QLatin1String(VirtualBox::ADD_SHARED));
        arguments.append(vminfo.id);
        arguments.append(QLatin1String(VirtualBox::NAME_OPT));
        arguments.append(QStringLiteral("home"));
        arguments.append(QLatin1String(VirtualBox::HOSTPATH));
        arguments.append(QDir::homePath());

        QProcess process;
        process.start(VirtualBox::vBoxManagePath(), arguments);
        bool success = process.waitForFinished();

        if (!success || process.exitCode() != 0) {
            return false;
        }
    }

    return true;
}

}
