/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "hemeratoolchain.h"

#include "hemeraconstants.h"
#include "hemerasdkmanager.h"
#include "hemeraqtversion.h"
#include "hemeratoolchain.h"

#include <hemeradevelopermodetarget.h>
#include <hemeradevelopermodetargetmanager.h>

#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/gccparser.h>
#include <projectexplorer/devicesupport/devicetypekitchooser.h>
#include <projectexplorer/devicesupport/devicemanager.h>
#include <qtsupport/baseqtversion.h>
#include <qtsupport/qtkitinformation.h>
#include <utils/detailswidget.h>
#include <utils/environment.h>
#include <utils/pathchooser.h>
#include <utils/qtcassert.h>

#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>

namespace Hemera {
namespace Internal {

// --------------------------------------------------------------------------
// Helpers:
// --------------------------------------------------------------------------

static const char sdkPathKeyC[] = "Hemera.HemeraToolChain.SdkPath";
static const char vmNameKeyC[]  = "Hemera.HemeraToolChain.VirtualMachineName";

// --------------------------------------------------------------------------
// HmrToolChain
// --------------------------------------------------------------------------

HemeraToolChain::HemeraToolChain(Detection autodetected, const DeveloperMode::Target::Ptr &nativeTarget, const QString &id)
    : GccToolChain(id, autodetected)
    , m_targetName(nativeTarget->name())
{
    const QString targetPath = nativeTarget->pathToScripts();

    const Utils::FileName gcc =
            Utils::FileName::fromString(targetPath + QStringLiteral("/bin/") +
                                        QLatin1String(Constants::HEMERA_WRAPPER_GCC));
    setDisplayName(QString::fromLatin1("GCC (%1)").arg(nativeTarget->name()));
    setCompilerCommand(gcc);
    setSupportedAbis(detectSupportedAbis());
    // We might hit a wall here, in case there's no supported ABIs (yet).
    if (!supportedAbis().isEmpty()) {
        setTargetAbi(supportedAbis().first());
    }
}

QString HemeraToolChain::targetName() const
{
    return m_targetName;
}

QString HemeraToolChain::type() const
{
    return QLatin1String(Constants::TOOLCHAIN_TYPE);
}

QString HemeraToolChain::makeCommand(const Utils::Environment &environment) const
{
    const QString make = QLatin1String(Constants::HEMERA_WRAPPER_MAKE);
    const QString makePath = environment.searchInPath(make).toString();
    if (!makePath.isEmpty())
        return makePath;

    return make;
}

QList<Utils::FileName> HemeraToolChain::suggestedMkspecList() const
{
    QList<Utils::FileName> mkSpecList = GccToolChain::suggestedMkspecList();
    if (mkSpecList.isEmpty())
        mkSpecList << Utils::FileName::fromString(QLatin1String("linux-g++"));
    return mkSpecList;
}

ProjectExplorer::ToolChain *HemeraToolChain::clone() const
{
    return new HemeraToolChain(*this);
}

ProjectExplorer::IOutputParser *HemeraToolChain::outputParser() const
{
    return new ProjectExplorer::GccParser;
}

QVariantMap HemeraToolChain::toMap() const
{
    QVariantMap data = GccToolChain::toMap();
    data.insert(QLatin1String(Constants::HEMERA_TARGET_NAME), m_targetName);
    return data;
}

bool HemeraToolChain::fromMap(const QVariantMap &data)
{
    if (!GccToolChain::fromMap(data))
        return false;

    m_targetName = data.value(QLatin1String(Constants::HEMERA_TARGET_NAME)).toString();
    return true;
}

QList<ProjectExplorer::Task> HemeraToolChain::validateKit(const ProjectExplorer::Kit *kit) const
{
    QList<ProjectExplorer::Task> result = GccToolChain::validateKit(kit);
    if (!result.isEmpty())
        return result;

    Core::Id type = ProjectExplorer::DeviceTypeKitInformation::deviceTypeId(kit);

    if (type == Constants::HEMERA_DEVICE_TYPE_EMULATOR && targetAbi().architecture() != ProjectExplorer::Abi::X86Architecture) {
        const QString message =
                tr("Hemera Toolchain '%1' can not be used for this emulator. %2, %3").arg(displayName(), type.toString(), ProjectExplorer::Abi::toString(targetAbi().architecture()));
        result << ProjectExplorer::Task(ProjectExplorer::Task::Error, message, Utils::FileName(), -1,
                       Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM));
    }

    QtSupport::BaseQtVersion *version = QtSupport::QtKitInformation::qtVersion(kit);

    if (!version) {
        const QString message =
                tr("No available Qt version found which can be used with toolchain '%1'.").arg(displayName());
        result << ProjectExplorer::Task(ProjectExplorer::Task::Error, message, Utils::FileName(), -1,
                       Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM));

    } else if (!Internal::HemeraSDKManager::validateKit(kit)) {
        const QString message =
                QCoreApplication::translate("ProjectExplorer::HemeraToolChain",
                                            "The toolchain '%1' does not match mersdk or qt version").
                                                                arg(displayName());
        result << ProjectExplorer::Task(ProjectExplorer::Task::Error, message, Utils::FileName(), -1,
                       Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM));
    }
    return result;
}

QList<ProjectExplorer::HeaderPath> HemeraToolChain::systemHeaderPaths(const QStringList &cxxflags, const Utils::FileName &sysRoot) const
{
    Q_UNUSED(cxxflags)
    if (m_headerPathsOnHost.isEmpty()) {
        m_headerPathsOnHost.append(ProjectExplorer::HeaderPath(sysRoot.toString() + QLatin1String("/usr/local/include"),
                                                               ProjectExplorer::HeaderPath::GlobalHeaderPath));
        m_headerPathsOnHost.append(ProjectExplorer::HeaderPath(sysRoot.toString() + QLatin1String("/usr/include"),
                                                               ProjectExplorer::HeaderPath::GlobalHeaderPath));
    }
    return m_headerPathsOnHost;
}


void HemeraToolChain::addToEnvironment(Utils::Environment &env) const
{
    GccToolChain::addToEnvironment(env);
    /*env.appendOrSet(QLatin1String(Constants::MER_SSH_TARGET_NAME),m_targetName);
    env.appendOrSet(QLatin1String(Constants::MER_SSH_SDK_TOOLS),compilerCommand().parentDir().toString());*/
}

// --------------------------------------------------------------------------
// HmrToolChainFactory
// --------------------------------------------------------------------------

HemeraToolChainFactory::HemeraToolChainFactory()
{
    setId(Constants::TOOLCHAIN_ID);
    setDisplayName(tr("Hemera"));
}

// Used by the ToolChainManager to restore user-generated tool chains
bool HemeraToolChainFactory::canRestore(const QVariantMap &data)
{
    const QString id = idFromMap(data);
    return id.startsWith(QLatin1String(Constants::TOOLCHAIN_ID) + QLatin1Char(':'));
}

ProjectExplorer::ToolChain *HemeraToolChainFactory::restore(const QVariantMap &data)
{
    QString targetName = data.value(QLatin1String(Constants::HEMERA_TARGET_NAME)).toString();
    DeveloperMode::Target::Ptr target = DeveloperMode::TargetManager::instance()->loadTarget(targetName);

    if (target.isNull()) {
        return 0;
    }

    HemeraToolChain *tc = new HemeraToolChain(ProjectExplorer::ToolChain::ManualDetection, target);
    if (tc->fromMap(data)) {
        return tc;
    }

    delete tc;
    return 0;
}

} // namespace Internal
} // namespace Hemera
