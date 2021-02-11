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

#ifndef HEMERATOOLCHAIN_H
#define HEMERATOOLCHAIN_H

#include "projectexplorer/abi.h"
#include "projectexplorer/customparser.h"
#include "projectexplorer/headerpath.h"
#include "projectexplorer/gcctoolchain.h"
#include "projectexplorer/toolchainconfigwidget.h"

#include <utils/fileutils.h>

#include <hemeraconstants.h>
#include <hemeradevelopermodetarget.h>

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
class QTextEdit;
class QComboBox;
class QPushButton;
QT_END_NAMESPACE

namespace ProjectExplorer { class AbiWidget; }
namespace Utils { class PathChooser; }

namespace Hemera {
namespace DeveloperMode {
class Target;
}

namespace Internal {

// --------------------------------------------------------------------------
// HmrToolChain
// --------------------------------------------------------------------------

class HemeraToolChain : public ProjectExplorer::GccToolChain
{
    Q_DECLARE_TR_FUNCTIONS(Hemera::HemeraToolchain)
public:
    HemeraToolChain(Detection autodetect, const DeveloperMode::Target::Ptr &nativeTarget, const QString &id = QLatin1String(Constants::TOOLCHAIN_ID));

    QString targetName() const;

    QString type() const;
    QVariantMap toMap() const;
    bool fromMap(const QVariantMap &data);

    QString makeCommand(const Utils::Environment &environment) const;

    ToolChain *clone() const;
    ProjectExplorer::IOutputParser *outputParser() const;
    QList<Utils::FileName> suggestedMkspecList() const;
    QList<ProjectExplorer::Task> validateKit(const ProjectExplorer::Kit *kit) const;

    QList<ProjectExplorer::HeaderPath> systemHeaderPaths(const QStringList &cxxflags,
                                                         const Utils::FileName &sysRoot) const;
    void addToEnvironment(Utils::Environment &env) const;
private:
    QString m_targetName;
    mutable QList<ProjectExplorer::HeaderPath> m_headerPathsOnHost;
};

class HemeraToolChainFactory : public ProjectExplorer::ToolChainFactory
{
    Q_OBJECT

public:
    HemeraToolChainFactory();

    virtual bool canCreate() override { return false; }

    // Used by the ToolChainManager to restore user-generated tool chains
    bool canRestore(const QVariantMap &data);
    ProjectExplorer::ToolChain *restore(const QVariantMap &data);
};

} // namespace Internal
} // namespace ProjectExplorer

#endif // HEMERATOOLCHAIN_H
