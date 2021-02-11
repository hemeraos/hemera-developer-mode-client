#ifndef HEMERA_H
#define HEMERA_H

#include "hemera_global.h"

#include <extensionsystem/iplugin.h>

namespace Hemera {
namespace Internal {

class HemeraPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Hemera.json")

public:
    HemeraPlugin();
    ~HemeraPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERA_H

