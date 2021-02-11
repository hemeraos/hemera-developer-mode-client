/*
 *
 */

#ifndef HEMERA_DEVELOPERMODE_TARGETMANAGER_H
#define HEMERA_DEVELOPERMODE_TARGETMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QFlags>
#include <QtCore/QHash>
#include <QtCore/QSettings>

#include "hemeradevelopermodeexport.h"

#include "hemeradevelopermodedevice.h"
#include "hemeradevelopermodeemulator.h"

namespace Hemera {
namespace DeveloperMode {

class Operation;

class Device;
class Emulator;
class Target;

class TargetManagerPrivate;
class HemeraDeveloperModeClient_EXPORT TargetManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TargetManager)

public:
    enum class EmulatorInstallMode : quint8 {
        NoMode = 0,
        KeepExistingVDI = 1 << 0,
        MoveVDI = 1 << 1
    };
    Q_DECLARE_FLAGS(EmulatorInstallModes, EmulatorInstallMode)
    Q_ENUMS(EmulatorInstallMode)

    static TargetManager * instance();

    virtual ~TargetManager();

    static QStringList availableDevices();
    static QHash< QString, QString > registeredEmulators();
    static QHash< QString, QString > availableVirtualMachines();

    static QString emulatorNameFromQuery(const QString &query);
    static QString deviceNameFromQuery(const QString &query);

    Target::Ptr loadTarget(const QString &targetName);

    Device::Ptr loadDevice(const QString &name);

    Emulator::Ptr loadEmulator(const QString &name);
    Emulator::Ptr loadEmulatorForDevice(const Device::Ptr &device);
    Emulator::Ptr loadEmulatorForDevice(const QString &deviceName);
    Emulator::Ptr runningEmulator();

    bool createStaticDevice(const QString &name, const QUrl &url, const QString &pathToHsdk);
    bool createKnownDevice(const QString &name, const QString &id, const QString &pathToHsdk);
    bool removeKnownDevice(const QString &name);

    static Operation *installEmulatorFromStartToken(const QString &name, const QString &startToken, const QUrl &startServer, const QString &pathToHsdk,
                                                    EmulatorInstallModes modes = EmulatorInstallMode::NoMode);
    static Operation *installEmulatorFromVDI(const QString &name, const QString &pathToVDI, const QString &pathToHsdk,
                                             EmulatorInstallModes modes = EmulatorInstallMode::NoMode);
    static Operation *updateEmulator(const QString &name, const QString &pathToHsdk);
    static Operation *removeEmulator(const QString &name, bool keepFiles);

    bool associate(const Device::Ptr &device, const Emulator::Ptr &emulator);

    static QSettings *settingsForTarget(const QString &group, const QString &name);

Q_SIGNALS:
    void dynamicTargetAnnounced(const QString &target);
    void dynamicTargetExpired(const QString &target);

    void updated();

private:
    TargetManager();

    TargetManagerPrivate * const d;

    friend class Target;
    friend class Device;
    friend class Emulator;
};

}
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Hemera::DeveloperMode::TargetManager::EmulatorInstallModes)

#endif // HEMERA_DEVELOPERMODE_TARGETMANAGER_H
