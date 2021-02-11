#ifndef HEMERA_DEVELOPERMODE_EMULATOR_H
#define HEMERA_DEVELOPERMODE_EMULATOR_H

#include <hemeradevelopermodetarget.h>

#include "hemeradevelopermodeexport.h"

namespace Hemera {
namespace DeveloperMode {

class Operation;

class EmulatorPrivate;
class HemeraDeveloperModeClient_EXPORT Emulator : public Hemera::DeveloperMode::Target
{
    Q_OBJECT
    Q_DISABLE_COPY(Emulator)
    Q_DECLARE_PRIVATE(Emulator)

    Q_PROPERTY(QStringList buildArchitectures READ buildArchitectures)
    Q_PROPERTY(QString     defaultBuildArchitecture READ defaultBuildArchitecture)
    Q_PROPERTY(bool running READ isRunning)

public:
    typedef QSharedPointer<Emulator> Ptr;
    typedef QSharedPointer<const Emulator> ConstPtr;

    explicit Emulator(const QString &id, TargetManager *parent);
    virtual ~Emulator();

    QStringList buildArchitectures() const;
    QString defaultBuildArchitecture() const;
    bool isRunning() const;

    Operation *start(bool headless = false);
    Operation *stop();

protected:
    virtual QSettings *settings() override;

    Ptr sharedFromThis();
    ConstPtr sharedFromThis() const;

private:
    friend class TargetManager;
    friend class TargetManagerPrivate;
};

}
}

#endif // HEMERA_DEVELOPERMODE_EMULATOR_H
