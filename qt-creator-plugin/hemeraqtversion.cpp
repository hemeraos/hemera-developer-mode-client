#include "hemeraqtversion.h"

#include "hemeraconstants.h"

#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodetarget.h>
#include <hemeradevelopermodetargetmanager.h>

#include <coreplugin/featureprovider.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <qtsupport/profilereader.h>
#include <qtsupport/qtsupportconstants.h>
#include <qtsupport/qtkitinformation.h>
#include <utils/qtcassert.h>

#include <proparser/profileevaluator.h>
#include <proparser/proitems.h>
#include <proparser/prowriter.h>
#include <proparser/qmakeglobals.h>
#include <proparser/qmakevfs.h>

#include <QtCore/QFileInfo>

namespace Hemera {
namespace Internal {

HemeraQtVersion::HemeraQtVersion()
    : BaseQtVersion()
{
}

HemeraQtVersion::HemeraQtVersion(const Utils::FileName &path, const DeveloperMode::Target::Ptr &nativeTarget, bool isAutodetected, const QString &autodetectionSource)
    : BaseQtVersion(path, isAutodetected, autodetectionSource)
    , m_targetName(nativeTarget->name())
{
    // We also need to find out about the ABI, if we got created here.
    DeveloperMode::Emulator::Ptr emu = nativeTarget.objectCast<DeveloperMode::Emulator>();
    if (!emu.isNull()) {
        // Aha.
        m_targetArchitecture = QStringLiteral("i586");
    } else {
        DeveloperMode::Device::Ptr device = nativeTarget.objectCast<DeveloperMode::Device>();
        if (device.isNull()) {
            qWarning() << "WTF???";
            return;
        }

        m_targetArchitecture = device->architecture();
    }

    populateAbiFromArchitecture();
}

HemeraQtVersion::~HemeraQtVersion()
{
}

void HemeraQtVersion::populateAbiFromArchitecture()
{
    m_abis.clear();

    if (m_targetArchitecture.contains(QStringLiteral("arm"))) {
        m_abis.append(ProjectExplorer::Abi(ProjectExplorer::Abi::ArmArchitecture, ProjectExplorer::Abi::LinuxOS, ProjectExplorer::Abi::GenericLinuxFlavor,
                                           ProjectExplorer::Abi::ElfFormat, 32));
    } else if (m_targetArchitecture.contains(QStringLiteral("mips"))) {
        m_abis.append(ProjectExplorer::Abi(ProjectExplorer::Abi::MipsArchitecture, ProjectExplorer::Abi::LinuxOS, ProjectExplorer::Abi::GenericLinuxFlavor,
                                           ProjectExplorer::Abi::ElfFormat, 32));
    } else if (m_targetArchitecture.contains(QStringLiteral("86_64"))) {
        m_abis.append(ProjectExplorer::Abi(ProjectExplorer::Abi::X86Architecture, ProjectExplorer::Abi::LinuxOS, ProjectExplorer::Abi::GenericLinuxFlavor,
                                           ProjectExplorer::Abi::ElfFormat, 64));
    } else if (m_targetArchitecture.contains(QStringLiteral("86"))) {
        m_abis.append(ProjectExplorer::Abi(ProjectExplorer::Abi::X86Architecture, ProjectExplorer::Abi::LinuxOS, ProjectExplorer::Abi::GenericLinuxFlavor,
                                           ProjectExplorer::Abi::ElfFormat, 32));
    } else {
        m_abis.append(ProjectExplorer::Abi(ProjectExplorer::Abi::UnknownArchitecture, ProjectExplorer::Abi::LinuxOS, ProjectExplorer::Abi::GenericLinuxFlavor,
                                           ProjectExplorer::Abi::ElfFormat, 32));
    }
}

QString HemeraQtVersion::targetName() const
{
    return m_targetName;
}

HemeraQtVersion *HemeraQtVersion::clone() const
{
    return new HemeraQtVersion(*this);
}

QString HemeraQtVersion::type() const
{
    return QLatin1String(Constants::HEMERA_QT);
}

QStringList HemeraQtVersion::warningReason() const
{
    QStringList ret = BaseQtVersion::warningReason();
    if (qtVersion() >= QtSupport::QtVersionNumber(5, 0, 0)) {
        if (qmlsceneCommand().isEmpty())
            ret << QCoreApplication::translate("QtVersion", "No qmlscene installed.");
    }
    return ret;
}

QList<ProjectExplorer::Abi> HemeraQtVersion::detectQtAbis() const
{
    return m_abis;
}

QString HemeraQtVersion::description() const
{
    return QStringLiteral("Hemera");
}

QString HemeraQtVersion::platformName() const
{
    return QLatin1String(Constants::HEMERA_QT_PLATFORM);
}

QString HemeraQtVersion::platformDisplayName() const
{
    return QLatin1String(Constants::HEMERA_QT_PLATFORM_TR);
}

QVariantMap HemeraQtVersion::toMap() const
{
    QVariantMap data = BaseQtVersion::toMap();
    data.insert(QLatin1String(Constants::HEMERA_TARGET_NAME), m_targetName);
    data.insert(QLatin1String(Constants::HEMERA_TARGET_ARCHITECTURE), m_targetArchitecture);
    return data;
}

void HemeraQtVersion::fromMap(const QVariantMap &data)
{
    BaseQtVersion::fromMap(data);
    m_targetName = data.value(QLatin1String(Constants::HEMERA_TARGET_NAME)).toString();
    m_targetArchitecture = data.value(QLatin1String(Constants::HEMERA_TARGET_ARCHITECTURE)).toString();

    populateAbiFromArchitecture();
}

QList<ProjectExplorer::Task> HemeraQtVersion::reportIssuesImpl(const QString &proFile, const QString &buildDir) const
{
    return QList<ProjectExplorer::Task>();
}

QList<ProjectExplorer::Task> HemeraQtVersion::validateKit(const ProjectExplorer::Kit *kit)
{
    QList<ProjectExplorer::Task> result = BaseQtVersion::validateKit(kit);
    if (!result.isEmpty()) {
        return result;
    }

    BaseQtVersion *version = QtSupport::QtKitInformation::qtVersion(kit);
    QTC_ASSERT(version == this, return result);

    ProjectExplorer::ToolChain *tc = ProjectExplorer::ToolChainKitInformation::toolChain(kit);

    if (!tc) {
        const QString message =
                tr("No available toolchains found to build for Qt version '%1'.").arg(version->displayName());
        result << ProjectExplorer::Task(ProjectExplorer::Task::Error, message, Utils::FileName(), -1,
                                        Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM));
    }

    return result;
}

/*void HemeraQtVersion::parseMkSpec(ProFileEvaluator *e) const
{
    // Fuck this. Creator is playing the bully and trying to get the most out of our Qt.
    // We don't really care about the mkspec here to be fairly honest, so we'll just redirect to
    // what we actually want.
    QMakeVfs vfs;
    ProFileGlobals option;
    option.setProperties(versionInfo());
    option.environment = qmakeRunEnvironment().toProcessEnvironment();
    QtSupport::ProMessageHandler msgHandler(true);
    QtSupport::ProFileCacheManager::instance()->incRefCount();
    QMakeParser parser(QtSupport::ProFileCacheManager::instance()->cache(), &vfs, &msgHandler);
    ProFileEvaluator evaluator(&option, &parser, &vfs, &msgHandler);
    qDebug() << "Loading named spec" << mkspecPath().toString() << evaluator.loadNamedSpec(QStringLiteral("/usr/lib/x86_64-linux-gnu/qt5/mkspecs/linux-g++"), false);

    BaseQtVersion::parseMkSpec(&evaluator);

    QtSupport::ProFileCacheManager::instance()->decRefCount();
}*/

Utils::FileName HemeraQtVersion::mkspecPath() const
{
    qDebug() << "OVERRIDING MKSPEC PATH!!";
    return Utils::FileName::fromString(QStringLiteral("/usr/lib/x86_64-linux-gnu/qt5/mkspecs/linux-g++"));
}

void HemeraQtVersion::addToEnvironment(const ProjectExplorer::Kit *k, Utils::Environment &env) const
{
    Q_UNUSED(k);
    //env.appendOrSet(QLatin1String(Constants::MER_SSH_PROJECT_PATH), QLatin1String("%{CurrentProject:Path}"));
    //env.appendOrSet(QLatin1String(Constants::MER_SSH_SDK_TOOLS),qmakeCommand().parentDir().toString());
}

Core::FeatureSet HemeraQtVersion::availableFeatures() const
{
    Core::FeatureSet features = BaseQtVersion::availableFeatures();
    features |= Core::FeatureSet(Constants::FEATURE_HEMERA);
    if(!qtAbis().contains(ProjectExplorer::Abi(QLatin1String("arm-linux-generic-elf-32bit"))))
        features |= Core::FeatureSet(Constants::FEATURE_EMULATOR);
    features |= Core::FeatureSet(QtSupport::Constants::FEATURE_MOBILE);
    features.remove(Core::Feature(QtSupport::Constants::FEATURE_QT_CONSOLE));
    return features;
}

Utils::Environment HemeraQtVersion::qmakeRunEnvironment() const
{
    Utils::Environment env = BaseQtVersion::qmakeRunEnvironment();
    //env.appendOrSet(QLatin1String(Constants::MER_SSH_TARGET_NAME),m_targetName);
    //env.appendOrSet(QLatin1String(Constants::MER_SSH_SDK_TOOLS),qmakeCommand().parentDir().toString());
    return env;
}

/////// FACTORY

HemeraQtVersionFactory::HemeraQtVersionFactory(QObject *parent)
    : QtVersionFactory(parent)
{

}

HemeraQtVersionFactory::~HemeraQtVersionFactory()
{

}

bool HemeraQtVersionFactory::canRestore(const QString &type)
{
    return type == QLatin1String(Constants::HEMERA_QT);
}

QtSupport::BaseQtVersion *HemeraQtVersionFactory::restore(const QString &type, const QVariantMap &data)
{
    if (!canRestore(type))
        return 0;
    HemeraQtVersion *v = new HemeraQtVersion;
    v->fromMap(data);
    return v;
}

int HemeraQtVersionFactory::priority() const
{
    // Rather high.
    return 100;
}

QtSupport::BaseQtVersion *HemeraQtVersionFactory::create(const Utils::FileName &qmakePath, ProFileEvaluator *evaluator,
                                                         bool isAutoDetected, const QString &autoDetectionSource)
{
    Q_UNUSED(qmakePath);
    Q_UNUSED(evaluator);
    Q_UNUSED(isAutoDetected);
    Q_UNUSED(autoDetectionSource);
    // Sorry, we simply can't.
    return 0;
}

} // namespace Internal
} // namespace Hemera

