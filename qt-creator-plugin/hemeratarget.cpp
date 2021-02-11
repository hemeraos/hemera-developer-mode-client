#include "hemeratarget.h"

#include "hemeradevicetester.h"
#include "hemerakitinformation.h"
#include "hemeraqtversion.h"
#include "hemeratoolchain.h"
#include "hyperspaceprocess.h"

#include <hemeradevelopermodetarget.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <debugger/debuggeritemmanager.h>
#include <debugger/debuggerkitinformation.h>
#include <qtsupport/qtversionmanager.h>
#include <qtsupport/qtkitinformation.h>
#include <qtsupport/qtversionfactory.h>
#include <projectexplorer/devicesupport/devicemanager.h>
#include <projectexplorer/devicesupport/deviceprocess.h>
#include <projectexplorer/devicesupport/deviceprocesslist.h>
#include <projectexplorer/toolchainmanager.h>
#include <utils/hostosinfo.h>
#include <utils/qtcassert.h>
#include <utils/algorithm.h>

namespace Hemera {
namespace Internal {

const char Delimiter0[] = "x--";
const char Delimiter1[] = "---";

static QString visualizeNull(QString s)
{
    return s.replace(QLatin1Char('\0'), QLatin1String("<null>"));
}

class HemeraDeviceProcessList : public ProjectExplorer::DeviceProcessList
{
public:
    HemeraDeviceProcessList(const ProjectExplorer::IDevice::ConstPtr &device, QObject *parent)
            : ProjectExplorer::DeviceProcessList(device, parent)
    {
    }

    virtual ~HemeraDeviceProcessList()
    {
    }

private:
    QString listProcessesCommandLine() const
    {
        return QStringLiteral("/usr/bin/hemera-list-processes");
    }

    virtual void doUpdate() override
    {
        // Invoke command
        ProjectExplorer::DeviceProcess *process = device()->createProcess(this);
        connect(process, &ProjectExplorer::DeviceProcess::finished, [this, process] {
            const QByteArray remoteStdout = process->readAllStandardOutput();
            const QString stdoutString
                    = QString::fromUtf8(remoteStdout.data(), remoteStdout.count());
            reportProcessListUpdated(buildProcessList(stdoutString));
            process->deleteLater();
        });
        process->start(listProcessesCommandLine());
    }

    virtual void doKillProcess(const ProjectExplorer::DeviceProcessItem &process) override
    {
        Q_UNUSED(process);
    }

    QList<ProjectExplorer::DeviceProcessItem> buildProcessList(const QString &listProcessesReply) const
    {
        QList<ProjectExplorer::DeviceProcessItem> processes;
        const QStringList lines = listProcessesReply.split(QString::fromLatin1(Delimiter0)
                + QString::fromLatin1(Delimiter1), QString::SkipEmptyParts);
        foreach (const QString &line, lines) {
            const QStringList elements = line.split(QLatin1Char('\n'));
            if (elements.count() < 4) {
                qDebug("%s: Expected four list elements, got %d. Line was '%s'.", Q_FUNC_INFO,
                       elements.count(), qPrintable(visualizeNull(line)));
                continue;
            }
            bool ok;
            const int pid = elements.first().mid(6).toInt(&ok);
            if (!ok) {
                qDebug("%s: Expected number in %s. Line was '%s'.", Q_FUNC_INFO,
                       qPrintable(elements.first()), qPrintable(visualizeNull(line)));
                continue;
            }
            QString command = elements.at(1);
            command.replace(QLatin1Char('\0'), QLatin1Char(' '));
            if (command.isEmpty()) {
                const QString &statString = elements.at(2);
                const int openParenPos = statString.indexOf(QLatin1Char('('));
                const int closedParenPos = statString.indexOf(QLatin1Char(')'), openParenPos);
                if (openParenPos == -1 || closedParenPos == -1)
                    continue;
                command = QLatin1Char('[')
                        + statString.mid(openParenPos + 1, closedParenPos - openParenPos - 1)
                        + QLatin1Char(']');
            }

            ProjectExplorer::DeviceProcessItem process;
            process.pid = pid;
            process.cmdLine = command;
            process.exe = elements.at(3);
            processes.append(process);
        }

        Utils::sort(processes);
        return processes;
    }
};

class HemeraTarget::Private
{
public:
    QString name;
    Hemera::DeveloperMode::Target::Ptr target;

    // It sucks, I know...
    QMetaObject::Connection onlineChangedConnection;
    QMetaObject::Connection developerModeControllerConnection;
};


// Origin is always manual, so they can be explicitly removed.
HemeraTarget::HemeraTarget(const DeveloperMode::Target::Ptr &target, Core::Id type, MachineType machineType)
    : ProjectExplorer::IDevice(type, Origin::ManuallyAdded, machineType, Core::Id::fromString(target->name()))
    , d(new Private)
{
    d->target = target;
    setDisplayName(target->name());

    // Regardless, we want to ensure our target is online. We don't care about the operation though.
    d->target->ensureOnline();

    auto internalTargetStateChanged = [this] {
        qDebug() << "internal target state changed";
        // Let's check.
        if (d->target->isOnline()) {
            if (d->target->hasAcquiredDeveloperModeController()) {
                setDeviceState(DeviceReadyToUse);
            } else {
                setDeviceState(DeviceConnected);
            }
        } else {
            setDeviceState(DeviceDisconnected);
        }
    };

    d->onlineChangedConnection = QObject::connect(target.data(), &Hemera::DeveloperMode::Target::onlineChanged,
                                                  ProjectExplorer::DeviceManager::instance(), internalTargetStateChanged);
    d->developerModeControllerConnection = QObject::connect(target.data(), &Hemera::DeveloperMode::Target::developerModeControllerChanged,
                                                            ProjectExplorer::DeviceManager::instance(), internalTargetStateChanged);

    internalTargetStateChanged();
}

HemeraTarget::~HemeraTarget()
{
    qDebug() << "Deleting Hemera target" << this;
    QObject::disconnect(d->onlineChangedConnection);
    QObject::disconnect(d->developerModeControllerConnection);
    delete d;
}

bool HemeraTarget::isCompatibleWith(const ProjectExplorer::Kit *k) const
{
    // Our Hemera Kit must match the device name.
    return HemeraKitInformation::targetName(k) == displayName();
}

QString HemeraTarget::displayType() const
{
    return tr("Hemera generic target");
}

ProjectExplorer::IDeviceWidget *HemeraTarget::createWidget()
{
    return nullptr;
}

QList<Core::Id> HemeraTarget::actionIds() const
{
    return QList<Core::Id>();
}

QString HemeraTarget::displayNameForActionId(Core::Id actionId) const
{
    Q_UNUSED(actionId);
    return QString();
}

void HemeraTarget::executeAction(Core::Id actionId, QWidget *parent)
{
    Q_UNUSED(actionId);
    Q_UNUSED(parent);
    return;
}

ProjectExplorer::DeviceProcessList *HemeraTarget::createProcessListModel(QObject *parent) const
{
    if (!target()->hasAcquiredDeveloperModeController()) {
        if (!target()->ensureDeveloperModeController(3000)->synchronize()) {
            return nullptr;
        }
    }
    return new HemeraDeviceProcessList(sharedFromThis(), parent);
}

ProjectExplorer::DeviceTester *HemeraTarget::createDeviceTester() const
{
    return new HemeraDeviceTester();
}

ProjectExplorer::DeviceProcess *HemeraTarget::createProcess(QObject *parent) const
{
    return new HyperspaceProcess(sharedFromThis(), parent);
}

ProjectExplorer::DeviceProcessSignalOperation::Ptr HemeraTarget::signalOperation() const
{
    return ProjectExplorer::DeviceProcessSignalOperation::Ptr();
}

ProjectExplorer::IDevice::Ptr HemeraTarget::clone() const
{
    qWarning() << "Trying to clone a target!!!";
    return ProjectExplorer::IDevice::Ptr();
}

QString HemeraTarget::qmlProfilerHost() const
{
    return QString();
}

DeveloperMode::Target::Ptr HemeraTarget::target() const
{
    return d->target;
}


// Handle other Qtc components originating from target
ProjectExplorer::Kit* HemeraTarget::createKit() const
{
    if (!target()) {
        return nullptr;
    }

    const QString sysroot = target()->pathToScripts();

    Utils::FileName path = Utils::FileName::fromString(sysroot);
    if (!path.toFileInfo().exists()) {
        qWarning() << "Sysroot does not exist" << sysroot;
        return 0;
    }

    ProjectExplorer::Kit *k = new ProjectExplorer::Kit();
    k->setAutoDetected(true);
    k->setUnexpandedDisplayName(QString::fromLatin1("%1").arg(target()->name()));
    //k->setIconPath(Utils::FileName::fromString(QLatin1String(Constants::MER_OPTIONS_CATEGORY_ICON)));

    if (machineType() == Emulator)  {
        ProjectExplorer::DeviceTypeKitInformation::setDeviceTypeId(k, Constants::HEMERA_DEVICE_TYPE_EMULATOR);
    } else {
        ProjectExplorer::DeviceTypeKitInformation::setDeviceTypeId(k, Constants::HEMERA_DEVICE_TYPE_DEVICE);
    }

    // TODO: Fixme when hdb is ready!
    const Utils::FileName gdbFileName =
            Utils::FileName::fromString(sysroot + QStringLiteral("/bin/") +
                                        QLatin1String(Constants::HEMERA_WRAPPER_GDB));

    Debugger::DebuggerItem debugger;
    debugger.setCommand(gdbFileName);
    debugger.setEngineType(Debugger::GdbEngineType);
    debugger.setDisplayName(QObject::tr("GDB for %1").arg(target()->name()));
    debugger.setAutoDetected(true);
    // Proxying is internal for us.
    debugger.setCanAttachDirectly(true);
    //debugger.setAbi(ProjectExplorer::Abi::abiFromTargetTriplet(m_gccMachineDump)); // TODO is this OK?
    QVariant id = Debugger::DebuggerItemManager::registerDebugger(debugger);
    Debugger::DebuggerKitInformation::setDebugger(k, id);

    HemeraKitInformation::setTargetName(k, target()->name());
    k->setUnexpandedDisplayName(tr("Hemera for %1").arg(target()->name()));
    return k;
}

HemeraQtVersion* HemeraTarget::createQtVersion() const
{
    const QString targetPath = target()->pathToScripts();

    const Utils::FileName qmake =
            Utils::FileName::fromString(targetPath + QStringLiteral("/bin/") +
                                        QLatin1String(Constants::HEMERA_WRAPPER_QMAKE));
    // Is there a qtversion present for this qmake?
    QtSupport::BaseQtVersion *qtv = QtSupport::QtVersionManager::qtVersionForQMakeBinary(qmake);
    if (qtv && !qtv->isValid()) {
        QtSupport::QtVersionManager::removeVersion(qtv);
        qtv = 0;
    }
    if (!qtv) {
        qtv = new HemeraQtVersion(qmake, target(), true, targetPath);
    }

    QTC_ASSERT(qtv && qtv->type() == QLatin1String(Constants::HEMERA_QT), return 0);

    HemeraQtVersion *hemeraQtv = static_cast<HemeraQtVersion *>(qtv);
    hemeraQtv->setUnexpandedDisplayName(
                QString::fromLatin1("Qt %1 on %2").arg(qtv->qtVersionString(),
                                                       target()->name()));
    return hemeraQtv;
}

HemeraToolChain* HemeraTarget::createToolChain() const
{
    const QString targetPath = target()->pathToScripts();

    const Utils::FileName gcc =
            Utils::FileName::fromString(targetPath + QStringLiteral("/bin/") +
                                        QLatin1String(Constants::HEMERA_WRAPPER_GCC));
    QList<ProjectExplorer::ToolChain *> toolChains = ProjectExplorer::ToolChainManager::toolChains();

    foreach (ProjectExplorer::ToolChain *tc, toolChains) {
        if (tc->compilerCommand() == gcc && tc->isAutoDetected()) {
            QTC_ASSERT(tc->type() == QLatin1String(Constants::TOOLCHAIN_TYPE), return 0);
        }
    }

    return new HemeraToolChain(ProjectExplorer::ToolChain::AutoDetection, target());
}

void HemeraTarget::registerTargetKits() const
{
    // Is there a kit already, first of all?
    for (ProjectExplorer::Kit *kit : ProjectExplorer::KitManager::kits()) {
        if (HemeraKitInformation::targetName(kit) == d->target->name()) {
            return;
        }
    }

    QScopedPointer<HemeraToolChain> toolchain(createToolChain());
    if (toolchain.isNull()) {
        return;
    }
    QScopedPointer<HemeraQtVersion> version(createQtVersion());
    if (version.isNull()) {
        return;
    }
    ProjectExplorer::Kit *kit = createKit();
    if (!kit) {
        return;
    }

    ProjectExplorer::ToolChainManager::registerToolChain(toolchain.data());
    QtSupport::QtVersionManager::addVersion(version.data());
    QtSupport::QtKitInformation::setQtVersion(kit, version.data());
    ProjectExplorer::ToolChainKitInformation::setToolChain(kit, toolchain.data());
    ProjectExplorer::DeviceKitInformation::setDevice(kit, sharedFromThis());
    ProjectExplorer::KitManager::registerKit(kit);
    toolchain.take();
    version.take();
}

} // namespace Internal
} // namespace Hemera

