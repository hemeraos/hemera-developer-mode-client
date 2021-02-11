#ifndef HEMERA_DEVELOPERMODE_EMULATOR_P_H
#define HEMERA_DEVELOPERMODE_EMULATOR_P_H

#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodeoperation.h>

#include "hemeradevelopermodetarget_p.h"

#include "hemeradevelopermodeexport.h"

namespace Hemera {
namespace DeveloperMode {

class HemeraDeveloperModeClient_EXPORT StartEmulatorOperation : public Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(StartEmulatorOperation)
public:
    explicit StartEmulatorOperation(Emulator::Ptr emulator, bool headless = false, QObject* parent = Q_NULLPTR);
    virtual ~StartEmulatorOperation();

protected:
    virtual void startImpl();

private Q_SLOTS:
    void performActualStart();

private:
    Emulator::Ptr m_emulator;
    bool m_headless;
};

class HemeraDeveloperModeClient_EXPORT StopEmulatorOperation : public Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(StopEmulatorOperation)
public:
    explicit StopEmulatorOperation(Emulator::Ptr emulator, QObject* parent = Q_NULLPTR);
    virtual ~StopEmulatorOperation();

protected:
    virtual void startImpl();

private:
    Emulator::Ptr m_emulator;
};

class EmulatorPrivate : public TargetPrivate
{
public:
    EmulatorPrivate(Target *q) : TargetPrivate(q) {}
};

}
}

#endif // HEMERA_DEVELOPERMODE_EMULATOR_P_H
