#ifndef HEMERATARGETFACTORY_H
#define HEMERATARGETFACTORY_H

#include <projectexplorer/devicesupport/idevicefactory.h>

namespace Hemera {
namespace Internal {

class HemeraTargetFactory : public ProjectExplorer::IDeviceFactory
{
    Q_OBJECT
public:
    explicit HemeraTargetFactory(QObject *parent = Q_NULLPTR);
    virtual ~HemeraTargetFactory();

    virtual QString displayNameForId(Core::Id type) const override final;
    virtual QList<Core::Id> availableCreationIds() const override final;

    virtual bool canCreate() const override final;
    virtual ProjectExplorer::IDevice::Ptr create(Core::Id id) const override final;

    virtual bool canRestore(const QVariantMap &map) const override final;
    virtual ProjectExplorer::IDevice::Ptr restore(const QVariantMap &map) const override final;

    static HemeraTargetFactory *instance();
};

}
}

#endif // HEMERATARGETFACTORY_H
