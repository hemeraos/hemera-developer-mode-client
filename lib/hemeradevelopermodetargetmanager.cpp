/*
 *
 */

#include "hemeradevelopermodetargetmanager.h"
#include "hemeradevelopermodetargetmanager_p.h"

#include "hemeradevelopermodetarget.h"
#include "hemeradevelopermodedevice_p.h"
#include "hemeradevelopermodeemulator_p.h"

#include "virtualboxcommands_p.h"
#include "hemeradevelopermodetarget_p.h"
#include "wrapperscripts.h"

#include <QtCore/QCache>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QGlobalStatic>
#include <QtCore/QPointer>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtCore/QDir>
#include <QtConcurrent/QtConcurrentRun>

#include <3rdparty/bzip2/bzlib.h>

// On OSX, everything is 64 by default.
#ifdef __APPLE__
#  define off64_t off_t
#  define fopen64 fopen
#endif

#define GET_SETTINGS QSettings settings(QStringLiteral("Hemera"), QStringLiteral("Targets"))

namespace Hemera {
namespace DeveloperMode {

static
bool myfeof ( FILE* f )
{
    qint32 c = fgetc ( f );
    if (c == EOF) return true;
    ungetc ( c, f );
    return false;
}

static
FILE* fopen_output_safely ( char* name, const char* mode )
{
#  if BZ_UNIX
    FILE*     fp;
    int fh;
    fh = open(name, O_WRONLY|O_CREAT|O_EXCL, S_IWUSR|S_IRUSR);
    if (fh == -1) return NULL;
    fp = fdopen(fh, mode);
    if (fp == NULL) close(fh);
    return fp;
#  else
    return fopen(name, mode);
#  endif
}

static void doDeleteLater(QObject *obj)
{
    obj->deleteLater();
}

Target::ConstPtr TargetManagerPrivate::fromRawPointer(const Target* target) const
{
    // The const_cast is safe, because we convert the Ptr back to a ConstPtr before returning it.
    return fromRawPointer(const_cast<Target *>(target));
}

Target::Ptr TargetManagerPrivate::fromRawPointer(Target* target) const
{
    if (qobject_cast<Device*>(target)) {
        return fromRawPointer(qobject_cast<Device*>(target)).objectCast<Target>();
    } else if (qobject_cast<Emulator*>(target)) {
        return fromRawPointer(qobject_cast<Emulator*>(target)).objectCast<Target>();
    }

    return Target::Ptr();
}

Emulator::ConstPtr TargetManagerPrivate::fromRawPointer(const Emulator* emulator) const
{
    // The const_cast is safe, because we convert the Ptr back to a ConstPtr before returning it.
    return fromRawPointer(const_cast<Emulator *>(emulator));
}

Emulator::Ptr TargetManagerPrivate::fromRawPointer(Emulator* emulator) const
{
    for (const Emulator::Ptr &emuPtr : emulatorsPool) {
        if (emuPtr == emulator) {
            return emuPtr;
        }
    }

    return Emulator::Ptr();
}

Device::ConstPtr TargetManagerPrivate::fromRawPointer(const Device* device) const
{
    // The const_cast is safe, because we convert the Ptr back to a ConstPtr before returning it.
    return fromRawPointer(const_cast<Device *>(device));
}

Device::Ptr TargetManagerPrivate::fromRawPointer(Device* device) const
{
    for (const Device::Ptr &devPtr : devicesPool) {
        if (devPtr == device) {
            return devPtr;
        }
    }

    return Device::Ptr();
}


InstallEmulatorOperation::InstallEmulatorOperation(const QString& name, const QString& filePath, TargetManager::EmulatorInstallModes mode,
                                                   const QString& pathToHsdk, QObject* parent)
    : Operation(parent)
    , m_name(name)
    , m_mode(mode)
    , m_hsdk(pathToHsdk)
    , m_filePath(filePath)

{
}

InstallEmulatorOperation::InstallEmulatorOperation(const QString &name, const QString &token, const QUrl &server, const QString& pathToHsdk,
                                                   TargetManager::EmulatorInstallModes mode, QObject* parent)
    : Operation(parent)
    , m_name(name)
    , m_mode(mode)
    , m_hsdk(pathToHsdk)
    , m_token(token)
    , m_server(server)
{
}

InstallEmulatorOperation::~InstallEmulatorOperation()
{
}

void InstallEmulatorOperation::setThingsToDo(int things)
{
    m_totalThings = things;
    m_leftThings = things;

    // We rollin'.
    Q_EMIT progress(0);
}

void InstallEmulatorOperation::oneThingLessToDo()
{
    --m_leftThings;
    Q_EMIT progress((100 * (m_totalThings - m_leftThings)) / m_totalThings);
}

void InstallEmulatorOperation::startImpl()
{
    // This is a nasty, blocking operation. But ya know, QtConcurrent, bitch.
    QtConcurrent::run([this] {
    VirtualBox::VirtualMachineInfo vminfo;
    // Basics: ports, shared directories, registration in settings
    int things = 3;
    // Does our emulator exist at all?
    bool update = TargetManager::registeredEmulators().contains(m_name);
    // Verify also the target vm exists...
    update &= TargetManager::availableVirtualMachines().values().contains(TargetManager::registeredEmulators().value(m_name));
    QString driveId;
    if (update) {
        vminfo = VirtualBox::virtualMachineInfo(TargetManager::registeredEmulators().value(m_name));
        if (!m_filePath.isEmpty()) {
            driveId = vminfo.mainDriveId;
            if (!driveId.isEmpty()) {
                // Detach and remove/delete
                things += 1;
            }
            // Add storage
            things += 1;
        }
    } else {
        // The whole package: create vm, create controller, attach disk.
        things += 3;
    }

    // We are "local" if we have a file, or if we don't have anything at all (simple parameters update).
    if (!m_filePath.isEmpty() || (m_filePath.isEmpty() && m_token.isEmpty())) {
        // No further things to do, unless...
        if (m_mode & TargetManager::EmulatorInstallMode::MoveVDI) {
            // Move file.
            things += 1;
        }

        // We ready.
        setThingsToDo(things);

        // First of all: is it a bz2 file? In that case, we have to decompress and move...
        if (m_filePath.endsWith(QStringLiteral("bz2"))) {
            // Let's go
            int bzerror;
            qint32  nBuf;
            char    buf[5000];
            uchar   unused[BZ_MAX_UNUSED];
            qint32   nUnused, streamNo;
            void*   unusedTmpV;
            uchar*  unusedTmp;
            FILE *bz2File;
            bz2File = fopen64(m_filePath.toLocal8Bit().constData(), "r");

            nUnused = 0;
            streamNo = 0;

            // Create file
            QFileInfo info(m_filePath);
            QString emulatorFileName = info.baseName() + QStringLiteral(".vdi");

            // Check our dir
            QDir driveDir(WrapperScripts::targetConfigurationPath(QStringLiteral("Emulators"), m_name) + QDir::separator() + QStringLiteral("drives"));
            if (!driveDir.exists()) {
                driveDir.mkpath(driveDir.absolutePath());
            }

            FILE *outVDIFile = fopen_output_safely(driveDir.absoluteFilePath(emulatorFileName).toLatin1().data(), "wb");
            if (!outVDIFile) {
                setFinishedWithError(tr("Decompression failed"), tr("Could not open target file for writing.").arg(bzerror));
                return;
            }

            SET_BINARY_MODE(bz2File);
            SET_BINARY_MODE(outVDIFile);

            while (true) {
                BZFILE *bzFileHandle = BZ2_bzReadOpen(&bzerror, bz2File, 0, 0, unused, nUnused);
                if (bzerror != BZ_OK) {
                    setFinishedWithError(tr("Decompression failed"), tr("Could not open file for decompression. Error code %1.").arg(bzerror));
                    return;
                }
                while (bzerror == BZ_OK) {
                    nBuf = BZ2_bzRead (&bzerror, bzFileHandle, buf, 5000);
                    if ((bzerror == BZ_OK || bzerror == BZ_STREAM_END) && nBuf > 0) {
                        fwrite ( buf, sizeof(uchar), nBuf, outVDIFile );
                        if (ferror(outVDIFile))  {
                            setFinishedWithError(tr("Decompression failed"), tr("Wrote inconsistent data to target file.").arg(bzerror));
                            return;
                        }
                    }
                }
                if (bzerror != BZ_STREAM_END) {
                    BZ2_bzReadClose(&bzerror, bzFileHandle);
                    setFinishedWithError(tr("Decompression failed"), tr("Error while decompressing. Error code %1.").arg(bzerror));
                    return;
                }

                BZ2_bzReadGetUnused ( &bzerror, bzFileHandle, &unusedTmpV, &nUnused );
                if (bzerror != BZ_OK) {
                    setFinishedWithError(tr("Decompression failed"), tr("Could not get unused bz2 data. Error code %1.").arg(bzerror));
                    return;
                }

                unusedTmp = (uchar*)unusedTmpV;
                for (qint32 i = 0; i < nUnused; i++) unused[i] = unusedTmp[i];

                BZ2_bzReadClose ( &bzerror, bzFileHandle );
                if (bzerror != BZ_OK) {
                    setFinishedWithError(tr("Decompression failed"), tr("Could not close bz2 data. Error code %1.").arg(bzerror));
                    return;
                }

                if (nUnused == 0 && myfeof(bz2File)) break;
            }

            fclose(outVDIFile);
            fflush(outVDIFile);
            m_filePath = driveDir.absoluteFilePath(emulatorFileName);
        }

        // Update or creation? Prepare bare VM and HDD in both cases.
        if (update) {
            // Do we actually need to update the disk?
            if (driveId.isEmpty() && m_filePath.isEmpty()) {
                setFinishedWithError(tr("No drive present"), tr("You requested an update for Emulator %1 without any new hard drive, "
                                                                "but the emulator has no hard drives at all!").arg(m_name));
                return;
            }

            // Now, for starters.
            if (!m_filePath.isEmpty() && !driveId.isEmpty()) {
                // Detach and (in case) delete
                if (!VirtualBox::detachLocalStorageFromEmulator(vminfo, !(m_mode & TargetManager::EmulatorInstallMode::KeepExistingVDI))) {
                    setFinishedWithError(tr("VirtualBox failed"), tr("Could not detach existing drive for Emulator %1.").arg(m_name));
                    return;
                }
                oneThingLessToDo();
            }
        } else {
            if (m_filePath.isEmpty()) {
                setFinishedWithError(tr("No drive present"), tr("You requested to install Emulator %1 without any new hard drive. "
                                                                "You need to specify one to create a working Emulator.").arg(m_name));
                return;
            }

            // We're starting brand new here.
            if (!VirtualBox::createVMForEmulator(m_name)) {
                setFinishedWithError(tr("VirtualBox failed"), tr("Could not create Virtual Machine for Emulator %1.").arg(m_name));
                return;
            }
            oneThingLessToDo();

            // Populate our info object.
            vminfo = VirtualBox::virtualMachineInfo(m_name);

            // Go for the storage controller creation
            if (!VirtualBox::createStorageControllerForEmulator(vminfo)) {
                setFinishedWithError(tr("VirtualBox failed"), tr("Could not create storage controller for %1.").arg(m_name));
                return;
            }
            oneThingLessToDo();
        }

        // Good! Now, do we need to attach our storage?
        if (!m_filePath.isEmpty()) {
            // Let's first have a look at the move part...
            if (m_mode & TargetManager::EmulatorInstallMode::MoveVDI) {
                // Move to a proper location
                QDir driveDir(WrapperScripts::targetConfigurationPath(QStringLiteral("Emulators"), m_name) + QDir::separator() + QStringLiteral("drives"));
                if (!driveDir.exists()) {
                    driveDir.mkpath(driveDir.absolutePath());
                }

                QFileInfo info(m_filePath);
                QString emulatorFileName = info.baseName() + QStringLiteral(".vdi");

                if (!QFile::rename(m_filePath, driveDir.absoluteFilePath(emulatorFileName))) {
                    setFinishedWithError(tr("Move failed"), tr("Could not move VDI file as requested."));
                    return;
                }

                m_filePath = driveDir.absoluteFilePath(emulatorFileName);
                oneThingLessToDo();
            }
            if (!VirtualBox::attachLocalStorageToEmulator(vminfo, m_filePath)) {
                setFinishedWithError(tr("VirtualBox failed"), tr("Could not attach %1 to emulator %2.").arg(m_filePath, m_name));
                return;
            }
            oneThingLessToDo();
        }
    } else {
        // TBD
        setFinished();
    }

    // Time for all the rest now.
    if (!VirtualBox::setSharedFoldersForEmulator(vminfo)) {
        setFinishedWithError(tr("VirtualBox failed"), tr("Could not create shared folders for emulator %1.").arg(m_name));
        return;
    }
    oneThingLessToDo();
    if (!VirtualBox::setPortsForEmulator(vminfo)) {
        setFinishedWithError(tr("VirtualBox failed"), tr("Could not create forwarded ports for emulator %1.").arg(m_name));
        return;
    }
    oneThingLessToDo();

    // Good. Create configuration
    GET_SETTINGS;
    settings.beginGroup(QStringLiteral("Emulators")); {
        settings.beginGroup(m_name); {
            settings.setValue(QStringLiteral("id"), vminfo.id);
        } settings.endGroup();
    } settings.endGroup();

    // Just to make sure.
    settings.sync();

    // Create wrapper scripts
    if (!WrapperScripts::createScripts(QStringLiteral("Emulators"), m_name, m_hsdk)) {
        setFinishedWithError(tr("Scripts creation failed"), tr("Could not create toolchain scripts for emulator %1.").arg(m_name));
        return;
    }

    oneThingLessToDo();

    // We're done!
    setFinished();
    });
}

RemoveEmulatorOperation::RemoveEmulatorOperation(const QString &name, bool keepFiles, QObject *parent)
    : Operation(parent)
    , m_name(name)
    , m_keepFiles(keepFiles)

{
}

RemoveEmulatorOperation::~RemoveEmulatorOperation()
{
}

void RemoveEmulatorOperation::startImpl()
{
    // Is it even real? Get the VM ID.
    QString emuId = TargetManager::registeredEmulators().value(TargetManager::emulatorNameFromQuery(m_name));
    if (emuId.isEmpty()) {
        setFinishedWithError(QStringLiteral("NotFound"), tr("There's no such emulator %1!").arg(m_name));
        return;
    }

    // This is a nasty, blocking operation. But ya know, QtConcurrent, bitch.
    QtConcurrent::run([this, emuId] {
        bool result = VirtualBox::deleteVM(emuId, !m_keepFiles);

        if (!result) {
            setFinishedWithError(QStringLiteral("VirtualBoxError"), tr("Could not unregister Virtual Machine for Emulator %1!").arg(m_name));
            return;
        }
        // Good. Wipe from configuration now...
        // Retrieve the actual name
        QString emuName = TargetManager::instance()->registeredEmulators().key(emuId);
        GET_SETTINGS;
        settings.beginGroup(QStringLiteral("Emulators")); {
            settings.remove(emuName);
        } settings.endGroup();

        // Just to make sure.
        settings.sync();

        // And we're set!
        setFinished();
    });
}

Device::Ptr TargetManagerPrivate::deviceFromCache(const QString &name)
{
    if (devicesPool.contains(name)) {
        return devicesPool.value(name);
    }

    Device::Ptr t;
    QByteArray authToken;

    // Static targets first
    GET_SETTINGS;
    settings.beginGroup(QStringLiteral("Devices")); {
        if (settings.childGroups().contains(name)) {
            settings.beginGroup(name); {
                // If we have the target URL, it's static.
                if (settings.contains(QStringLiteral("url"))) {
                    t = Device::Ptr(new Device(settings.value(QStringLiteral("url")).toUrl(), q), doDeleteLater);
                } else if (settings.contains(QStringLiteral("id"))) {
                    // If we have the ID, it's dynamic.
                    t = Device::Ptr(new Device(settings.value(QStringLiteral("id")).toString(), q), doDeleteLater);
                }
                authToken = settings.value(QStringLiteral("authToken")).toByteArray();
            } settings.endGroup();
        }
    } settings.endGroup();

    if (!t.isNull()) {
        // Set its name.
        t->d_func()->name = name;
        t->d_func()->authToken = authToken;
        // Add to pool.
        devicesPool.insert(name, t);
    }

    return t;
}

Emulator::Ptr TargetManagerPrivate::emulatorFromCache(const QString &name)
{
    if (emulatorsPool.contains(name)) {
        return emulatorsPool.value(name);
    }

    Emulator::Ptr t;

    // Static targets first
    GET_SETTINGS;
    settings.beginGroup(QStringLiteral("Emulators")); {
        if (settings.childGroups().contains(name)) {
            settings.beginGroup(name); {
                if (settings.contains(QStringLiteral("id"))) {
                    t = Emulator::Ptr(new Emulator(settings.value(QStringLiteral("id")).toString(), q), doDeleteLater);
                }
            } settings.endGroup();
        }
    } settings.endGroup();

    if (!t.isNull()) {
        // Set its name.
        t->d_func()->name = name;
        // Add to pool.
        emulatorsPool.insert(name, t);
    }

    return t;
}


class TargetManagerHelper
{
public:
    TargetManagerHelper() : q(0) {}
    ~TargetManagerHelper() {
        delete q;
    }
    TargetManager *q;
};

Q_GLOBAL_STATIC(TargetManagerHelper, s_globalTargetManager)

static void cleanup_targetmanager()
{
    if (s_globalTargetManager->q) {
        s_globalTargetManager->q->deleteLater();
    }
}

TargetManager * TargetManager::instance()
{
    if (!s_globalTargetManager()->q) {
        new TargetManager();
    }

    return s_globalTargetManager()->q;
}


TargetManager::TargetManager()
    : QObject()
    , d(new TargetManagerPrivate(this))
{
    Q_ASSERT(!s_globalTargetManager->q);
    s_globalTargetManager->q = this;

    // Add a post routine for correct deletion of QNAM.
    qAddPostRoutine(cleanup_targetmanager);
}

TargetManager::~TargetManager()
{
    delete d;
}

QStringList TargetManager::availableDevices()
{
    // Fetch static targets from the configuration file
    GET_SETTINGS;

    QStringList devices;
    settings.beginGroup(QStringLiteral("Devices")); {
        devices = settings.childGroups();
    } settings.endGroup();

    return devices;
}

QHash< QString, QString > TargetManager::availableVirtualMachines()
{
    return VirtualBox::availableVirtualMachines();
}

QHash< QString, QString > TargetManager::registeredEmulators()
{
    QHash< QString, QString > emulators;

    GET_SETTINGS;
    settings.beginGroup(QStringLiteral("Emulators")); {
        for (const QString &group : settings.childGroups()) {
            settings.beginGroup(group); {
                if (settings.contains(QStringLiteral("id"))) {
                    emulators.insert(group, settings.value(QStringLiteral("id")).toString());
                }
            } settings.endGroup();
        }
    } settings.endGroup();

    return emulators;
}


QString TargetManager::emulatorNameFromQuery(const QString& query)
{
    QHash< QString, QString > vms = availableVirtualMachines();

    if (registeredEmulators().contains(query)) {
        return query;
    } else if (vms.contains(query)) {
        return registeredEmulators().key(vms.value(query));
    } else if (vms.values().contains(query)) {
        return registeredEmulators().key(query);
    }

    return QString();
}

QString TargetManager::deviceNameFromQuery(const QString& query)
{
    if (query.isEmpty()) {
        return QString();
    }

    GET_SETTINGS;
    settings.beginGroup(QStringLiteral("Devices")); {
        if (settings.childGroups().contains(query)) {
            return query;
        }
        for (const QString &group : settings.childGroups()) {
            settings.beginGroup(group); {
                if (settings.contains(QStringLiteral("id")) && settings.value(QStringLiteral("id")).toString() == query) {
                    return group;
                }
                QUrl url = QUrl(query);
                if (settings.contains(QStringLiteral("url")) && settings.value(QStringLiteral("url")).toUrl() == url && url.isValid() && !url.scheme().isEmpty()) {
                    return group;
                }
            } settings.endGroup();
        }
    } settings.endGroup();

    return QString();
}

Operation* TargetManager::installEmulatorFromStartToken(const QString &name, const QString &startToken,
                                                        const QUrl &startServer, const QString& pathToHsdk, EmulatorInstallModes modes)
{
    // TODO: When Start will be ready.
    return Q_NULLPTR;
}

Operation* TargetManager::installEmulatorFromVDI(const QString &name, const QString &pathToVDI, const QString& pathToHsdk, EmulatorInstallModes modes)
{
    return new InstallEmulatorOperation(name, pathToVDI, modes, pathToHsdk, Q_NULLPTR);
}

Operation* TargetManager::removeEmulator(const QString& name, bool keepFiles)
{
    return new RemoveEmulatorOperation(name, keepFiles, Q_NULLPTR);
}

Operation* TargetManager::updateEmulator(const QString &name, const QString& pathToHsdk)
{
    return new InstallEmulatorOperation(name, QString(), EmulatorInstallMode::NoMode, pathToHsdk, Q_NULLPTR);
}


Emulator::Ptr TargetManager::runningEmulator()
{
    QStringList vms = VirtualBox::runningVirtualMachines();

    for (const QString &vm : vms) {
        if (registeredEmulators().values().contains(vm)) {
            return d->emulatorFromCache(registeredEmulators().key(vm));
        }
    }

    return Emulator::Ptr();
}

bool TargetManager::createStaticDevice(const QString& name, const QUrl &url, const QString& pathToHsdk)
{
    GET_SETTINGS;

    // Change the configuration
    settings.beginGroup(QStringLiteral("Devices")); {
        settings.beginGroup(name); {
            settings.setValue(QStringLiteral("url"), url);
        } settings.endGroup();
    } settings.endGroup();

    // Just to make sure.
    settings.sync();

    // Create wrapper scripts
    if (!WrapperScripts::createScripts(QStringLiteral("Devices"), name, pathToHsdk)) {
        return false;
    }

    // Done
    return true;
}

bool TargetManager::createKnownDevice(const QString& name, const QString& id, const QString& pathToHsdk)
{
    GET_SETTINGS;

    // Change the configuration
    settings.beginGroup(QStringLiteral("Devices")); {
        settings.beginGroup(name); {
            settings.setValue(QStringLiteral("id"), id);
        } settings.endGroup();
    } settings.endGroup();

    // Just to make sure.
    settings.sync();

    // Create wrapper scripts
    if (!WrapperScripts::createScripts(QStringLiteral("Devices"), name, pathToHsdk)) {
        return false;
    }

    // Done
    return true;
}

QSettings *TargetManager::settingsForTarget(const QString& group, const QString& name)
{
    QSettings *settings = new QSettings(QStringLiteral("Hemera"), QStringLiteral("Targets"));
    settings->beginGroup(group);
    settings->beginGroup(name);

    return settings;
}

Emulator::Ptr TargetManager::loadEmulator(const QString &name)
{
    return d->emulatorFromCache(name);
}

Emulator::Ptr TargetManager::loadEmulatorForDevice(const QString &deviceName)
{
    // Ok, it's fine.
    GET_SETTINGS;

    // Change the configuration
    settings.beginGroup(QStringLiteral("Devices")); {
        settings.beginGroup(deviceName); {
            if (settings.contains(QStringLiteral("emulator"))) {
                return loadEmulator(settings.value(QStringLiteral("emulator")).toString());
            }
        } settings.endGroup();
    } settings.endGroup();

    return Emulator::Ptr();
}

Emulator::Ptr TargetManager::loadEmulatorForDevice(const Device::Ptr &device)
{
    return loadEmulatorForDevice(device->name());
}

Device::Ptr TargetManager::loadDevice(const QString &name)
{
    return d->deviceFromCache(name);
}

Target::Ptr TargetManager::loadTarget(const QString &targetName)
{
    // Strive to load any emulator or device
    Target::Ptr t;
    t = d->deviceFromCache(targetName);
    if (t) {
        return t;
    }
    t = d->emulatorFromCache(targetName);
    return t;
}


bool TargetManager::removeKnownDevice(const QString& name)
{
    // Check if we need an update first
    GET_SETTINGS;
    settings.beginGroup(QStringLiteral("Devices")); {
        if (settings.childGroups().contains(name)) {
            settings.remove(name);
        }
    } settings.endGroup();

    // Done
    return true;
}

bool TargetManager::associate(const Device::Ptr &device, const Emulator::Ptr &emulator)
{
    // For starters, are we compatible?
    if (!device->architecture().isEmpty() && !emulator->buildArchitectures().isEmpty()) {
        // Check
        if (!emulator->buildArchitectures().contains(device->architecture())) {
            return false;
        }
    }

    // Ok, it's fine.
    GET_SETTINGS;

    // Change the configuration
    settings.beginGroup(QStringLiteral("Devices")); {
        settings.beginGroup(device->name()); {
            settings.setValue(QStringLiteral("emulator"), emulator->name());
        } settings.endGroup();
    } settings.endGroup();

    // Just to make sure.
    settings.sync();

    // Done
    return true;
}

}
}
