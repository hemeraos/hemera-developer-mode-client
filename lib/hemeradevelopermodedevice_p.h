#ifndef HEMERA_DEVELOPERMODE_DEVICE_P_H
#define HEMERA_DEVELOPERMODE_DEVICE_P_H

#include <hemeradevelopermodedevice.h>
#include "hemeradevelopermodetarget_p.h"

#include "hemeradevelopermodeexport.h"

namespace Hemera {
namespace DeveloperMode {

class DevicePrivate : public TargetPrivate
{
public:
    DevicePrivate(Target *q) : TargetPrivate(q) {}
};

}
}

#endif // HEMERA_DEVELOPERMODE_DEVICE_P_H
