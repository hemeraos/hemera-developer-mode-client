#include "hemeradevelopermodeemulator_p.h"

#include "hemeradevelopermodetargetmanager.h"

#include "virtualboxcommands_p.h"
#include "wrapperscripts.h"
#include "hemeradevelopermodetargetmanager_p.h"

#include <QtConcurrent/QtConcurrentRun>

namespace Hemera {
namespace DeveloperMode {

StartEmulatorOperation::StartEmulatorOperation(Emulator::Ptr emulator, bool headless, QObject* parent)
    : Operation(parent)
    , m_emulator(emulator)
    , m_headless(headless)
{
}

StartEmulatorOperation::~StartEmulatorOperation()
{
}

void StartEmulatorOperation::startImpl()
{
    if (TargetManager::instance()->runningEmulator() && TargetManager::instance()->runningEmulator() != m_emulator) {
        // We need to stop the running emulator first.
        connect(new StopEmulatorOperation(TargetManager::instance()->runningEmulator()), &Operation::finished, this, &StartEmulatorOperation::performActualStart);
    } else {
        performActualStart();
    }
}

void StartEmulatorOperation::performActualStart()
{
    if (TargetManager::instance()->runningEmulator() && TargetManager::instance()->runningEmulator() != m_emulator) {
        setFinishedWithError(tr("Emulator running"), tr("Another emulator is currently running, and cannot be stopped."));
        return;
    }

    // Threaded superpowers
    QtConcurrent::run([this] {
        if (VirtualBox::startVirtualMachine(m_emulator->id(), m_headless)) {
            setFinished();
        } else {
            setFinishedWithError(tr("Could not start emulator"), tr("Emulator %1 could not be started.").arg(m_emulator->name()));
        }
    });
}

StopEmulatorOperation::StopEmulatorOperation(Emulator::Ptr emulator, QObject* parent)
    : Operation(parent)
    , m_emulator(emulator)
{
}

StopEmulatorOperation::~StopEmulatorOperation()
{
}

void StopEmulatorOperation::startImpl()
{
    // TODO: In the future, try shutting down gracefully before killing the VM.
    // Threaded superpowers
    QtConcurrent::run([this] {
        if (VirtualBox::killVirtualMachine(m_emulator->id())) {
            setFinished();
        } else {
            setFinishedWithError(tr("Could not stop emulator"), tr("Emulator %1 could not be stopped.").arg(m_emulator->name()));
        }
    });
}


Emulator::Emulator(const QString &id, TargetManager* parent)
    : Target(*new EmulatorPrivate(this), QUrl(QStringLiteral("https://127.0.0.1:8080/")), parent, id)
{
    setTypeName(QStringLiteral("Emulators"));
}

Emulator::~Emulator()
{
}

Operation* Emulator::start(bool headless)
{
    return new StartEmulatorOperation(sharedFromThis(), headless, this);
}

Operation* Emulator::stop()
{
    return new StopEmulatorOperation(sharedFromThis());
}

QStringList Emulator::buildArchitectures() const
{
    Q_D(const Emulator);
    return d->buildArchitectures;
}

QString Emulator::defaultBuildArchitecture() const
{
    Q_D(const Emulator);
    return d->defaultBuildArchitecture;
}

bool Emulator::isRunning() const
{
    return this == TargetManager::instance()->runningEmulator();
}

QSettings *Emulator::settings()
{
    return TargetManager::settingsForTarget(QStringLiteral("Emulators"), name());
}

Emulator::ConstPtr Emulator::sharedFromThis() const
{
    return TargetManager::instance()->d->fromRawPointer(this);
}

Emulator::Ptr Emulator::sharedFromThis()
{
    return TargetManager::instance()->d->fromRawPointer(this);
}

}
}
