#ifndef HEMERA_DEVELOPERMODE_DEVICE_H
#define HEMERA_DEVELOPERMODE_DEVICE_H

#include "hemeradevelopermodetarget.h"

#include "hemeradevelopermodeemulator.h"

#include "hemeradevelopermodeexport.h"

namespace Hemera {
namespace DeveloperMode {

class Operation;

class Emulator;

class DevicePrivate;
class HemeraDeveloperModeClient_EXPORT Device : public Hemera::DeveloperMode::Target
{
    Q_OBJECT
    Q_DISABLE_COPY(Device)
    Q_DECLARE_PRIVATE(Device)

    Q_PROPERTY(QString          applianceName       READ applianceName            STORED true)
    Q_PROPERTY(bool             isProductionDevice  READ isProductionDevice       STORED true)
    Q_PROPERTY(QString          architecture        READ architecture             STORED true)

public:
    enum class DeviceType {
        GenericOrUnknown = 0,
        Headless,
        HeadlessEvaluationBoard,
        TabletLike,
        PhoneLike,
        MultiDisplay,
        Automotive
    };

    typedef QSharedPointer<Device> Ptr;
    typedef QSharedPointer<const Device> ConstPtr;

    explicit Device(const QString &id, Hemera::DeveloperMode::TargetManager *parent);
    explicit Device(Hemera::DeveloperMode::Transport *transport, Hemera::DeveloperMode::TargetManager *parent, const QString &id = QString());
    explicit Device(const QUrl &url, Hemera::DeveloperMode::TargetManager *parent);

    virtual ~Device();

    QString applianceName() const;
    bool isProductionDevice() const;
    QString architecture() const;

    Emulator::Ptr associatedEmulator();

    Operation *startAssociatedEmulator(bool headless = false);

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

#endif // HEMERA_DEVELOPERMODE_DEVICE_H
