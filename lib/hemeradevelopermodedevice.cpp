#include "hemeradevelopermodedevice_p.h"

#include "hemeradevelopermodetarget_p.h"
#include "hemeradevelopermodetargetmanager.h"
#include "hemeradevelopermodeemulator.h"
#include "wrapperscripts.h"
#include "hemeradevelopermodetargetmanager_p.h"

namespace Hemera {
namespace DeveloperMode {

Device::Device(const QUrl& url, TargetManager* parent)
    : Target(*new DevicePrivate(this), url, parent)
{
    setTypeName(QStringLiteral("Devices"));
}

Device::Device(Transport* transport, TargetManager* parent, const QString& id)
    : Target(*new DevicePrivate(this), transport, parent, id)
{
    setTypeName(QStringLiteral("Devices"));
}

Device::Device(const QString& id, TargetManager* parent)
    : Target(*new DevicePrivate(this), id, parent)
{
    setTypeName(QStringLiteral("Devices"));
}

Device::~Device()
{
}

QString Device::applianceName() const
{
    Q_D(const Device);
    return d->applianceName;
}

QString Device::architecture() const
{
    Q_D(const Device);
    return d->architecture;
}

bool Device::isProductionDevice() const
{
    Q_D(const Device);
    return d->isProductionBoard;
}

Emulator::Ptr Device::associatedEmulator()
{
    return TargetManager::instance()->loadEmulatorForDevice(sharedFromThis());
}

Operation* Device::startAssociatedEmulator(bool headless)
{
    Emulator::Ptr emulator = TargetManager::instance()->loadEmulatorForDevice(sharedFromThis());
    if (!emulator) {
        return Q_NULLPTR;
    }

    return emulator->start(headless);
}

QSettings *Device::settings()
{
    return TargetManager::settingsForTarget(QStringLiteral("Devices"), name());
}

Device::ConstPtr Device::sharedFromThis() const
{
    return TargetManager::instance()->d->fromRawPointer(this);
}

Device::Ptr Device::sharedFromThis()
{
    return TargetManager::instance()->d->fromRawPointer(this);
}

}
}
