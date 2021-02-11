#include "wrapperscripts.h"

#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodetarget.h>

#include "hemeradevelopermodetargetmanager.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>

// Script wrappers
#ifdef Q_OS_WIN
#define SCRIPT_EXTENSION ".cmd"
#else // Q_OS_WIN
#define SCRIPT_EXTENSION ""
#endif // Q_OS_WIN

const char HEMERA_WRAPPER_RPMBUILD[] = "rpmbuild" SCRIPT_EXTENSION;
const char HEMERA_WRAPPER_FULL_BUILD[] = "full-build" SCRIPT_EXTENSION;
const char HEMERA_WRAPPER_CMAKE[] = "cmake" SCRIPT_EXTENSION;
const char HEMERA_WRAPPER_QMAKE[] = "qmake" SCRIPT_EXTENSION;
const char HEMERA_WRAPPER_MAKE[] = "make" SCRIPT_EXTENSION;
const char HEMERA_WRAPPER_GCC[] = "gcc" SCRIPT_EXTENSION;
const char HEMERA_WRAPPER_GDB[] = "gdb" SCRIPT_EXTENSION;

const char* wrapperScripts[] =
{
    HEMERA_WRAPPER_QMAKE,
    HEMERA_WRAPPER_CMAKE,
    HEMERA_WRAPPER_MAKE,
    HEMERA_WRAPPER_GCC,
    HEMERA_WRAPPER_GDB,
    HEMERA_WRAPPER_RPMBUILD,
    HEMERA_WRAPPER_FULL_BUILD
};

enum OsType { OsTypeWindows, OsTypeLinux, OsTypeMac, OsTypeOtherUnix, OsTypeOther };

OsType hostOs()
{
#if defined(Q_OS_WIN)
    return OsTypeWindows;
#elif defined(Q_OS_LINUX)
    return OsTypeLinux;
#elif defined(Q_OS_MAC)
    return OsTypeMac;
#elif defined(Q_OS_UNIX)
    return OsTypeOtherUnix;
#else
    return OsTypeOther;
#endif
}

QString targetConfigurationPath()
{
    QString targetPath = QStringLiteral("%1%2Hemera").arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation),
                                                          QDir::separator());
    return targetPath;
}

QString WrapperScripts::targetConfigurationPath(const QString &group, const QString &name)
{
    if (QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation).isEmpty()) {
        qWarning() << "Could not find data location for generic data directory!!";
        return QString();
    }

    QString targetPath = QStringLiteral("%1%2%3%2%4").arg(::targetConfigurationPath(), QDir::separator(), group, name);
    return targetPath;
}

bool WrapperScripts::createScript(const QString &group, const QString &name, const QString &pathToHsdk, int scriptIndex)
{
    bool ok = false;
    QString targetPath = targetConfigurationPath(group, name);
    const char* wrapperScriptCopy = wrapperScripts[scriptIndex];
    const QFile::Permissions rwrw = QFile::ReadOwner|QFile::ReadUser|QFile::ReadGroup
            |QFile::WriteOwner|QFile::WriteUser|QFile::WriteGroup;
    const QFile::Permissions rwxrwx = rwrw|QFile::ExeOwner|QFile::ExeUser|QFile::ExeGroup;

    QString wrapperScriptCommand = QLatin1String(wrapperScriptCopy);
    if (hostOs() == OsTypeWindows)
        wrapperScriptCommand.chop(4); // remove the ".cmd"

    const QString scriptCopyPath = targetPath + QStringLiteral("/bin/") + QLatin1String(wrapperScriptCopy);
    QDir targetDir(targetPath + QDir::separator() + QStringLiteral("bin"));
    if (!targetDir.exists()) {
        targetDir.mkpath(targetDir.absolutePath());
    }
    const QString targetName = name;
    targetDir.cdUp();
    const QString merDevToolsDir = QDir::toNativeSeparators(targetDir.canonicalPath());
    QFile script(scriptCopyPath);
    ok = script.open(QIODevice::WriteOnly);
    if (!ok) {
        qWarning() << "Could not open script" << scriptCopyPath;
        return false;
    }

    QString targetSwitch;
    if (group == QStringLiteral("Emulators")) {
        targetSwitch = QStringLiteral("-e %1").arg(name);
    } else {
        targetSwitch = QStringLiteral("-d %1").arg(name);
    }

    QString scriptContent;

    if (hostOs() == OsTypeWindows) {
        scriptContent += QLatin1String("@echo off\nSetLocal EnableDelayedExpansion\n");
        scriptContent += QLatin1String("set ARGUMENTS=\nFOR %%a IN (%*) DO set ARGUMENTS=!ARGUMENTS! ^ %%a\n");
        // Add any environment variable here.
        scriptContent += QLatin1String("set ") +
                QLatin1String("HEMERA_DEVICE_NAME") +
                QLatin1Char('=') + targetName + QLatin1Char('\n');
        scriptContent += QLatin1String("SetLocal DisableDelayedExpansion\n");
        QString hsdkMainCommand;
        if (wrapperScriptCommand != QStringLiteral("gdb")) {
            hsdkMainCommand = QLatin1String("invoke");
            wrapperScriptCommand += QLatin1Char(' ');
        } else {
            hsdkMainCommand = QStringLiteral("gdb");
            wrapperScriptCommand = QString();
        }
        scriptContent += QLatin1Char('"') +
                QDir::toNativeSeparators(pathToHsdk) + QLatin1String("\" ") + hsdkMainCommand +
                QLatin1Char(' ') + QStringLiteral("--fail-on-offline") + QLatin1Char(' ') + targetSwitch + QLatin1String(" \"") +
                wrapperScriptCommand + QLatin1String("%ARGUMENTS%\"\n");
    }

    if (hostOs() == OsTypeOtherUnix || hostOs() == OsTypeLinux || hostOs() == OsTypeMac) {
        scriptContent += QLatin1String("#!/bin/sh\n");
        scriptContent += QLatin1String("export  ") +
                QLatin1String("HEMERA_DEVICE_NAME") +
                QLatin1Char('=') + targetName + QLatin1Char('\n');
        scriptContent += QLatin1String("exec ");
        QString hsdkMainCommand;
        if (wrapperScriptCommand != QStringLiteral("gdb")) {
            hsdkMainCommand = QLatin1String("invoke");
            wrapperScriptCommand += QLatin1Char(' ');
        } else {
            hsdkMainCommand = QStringLiteral("gdb");
            wrapperScriptCommand = QString();
        }
        scriptContent += QLatin1Char('"') +
                QDir::toNativeSeparators(pathToHsdk) + QLatin1String("\" ") + hsdkMainCommand +
                QLatin1Char(' ') + QStringLiteral("--fail-on-offline") + QLatin1Char(' ') + targetSwitch + QLatin1String(" \"") +
                wrapperScriptCommand + QLatin1String("${@}\"");
    }


    ok = script.write(scriptContent.toUtf8()) != -1;
    if (!ok) {
        qWarning() << "Could not write script" << scriptCopyPath;
        return false;
    }

    ok = QFile::setPermissions(scriptCopyPath, rwxrwx);
    if (!ok) {
        qWarning() << "Could not set file permissions on script" << scriptCopyPath;
        return false;
    }
    return ok;
}

bool WrapperScripts::createScripts(const QString &group, const QString &name, const QString &pathToHsdk)
{
    QString targetPath = targetConfigurationPath(group, name);
    QDir targetDir(targetPath);
    if (!targetDir.exists() && !targetDir.mkpath(targetPath)) {
        qWarning() << "Could not create target directory." << targetDir;
        return false;
    }
    bool result = true;
    for (size_t i = 0; i < sizeof(wrapperScripts) / sizeof(wrapperScripts[0]); ++i)
         result &= createScript(group, name, pathToHsdk, i);

    // Once one creates the scripts, it's also a good idea to refresh the cache.
    // KIDDING BITCH. This fucks up pretty much all of our multithreading design. Besides, that op does not really do anything now.
    //Hemera::DeveloperMode::TargetManager::instance()->loadTarget(name)->updateLocalTargetCache();

    return result;
}

void WrapperScripts::deleteScripts(const QString& group, const QString& name)
{
    QString targetPath = targetConfigurationPath(group, name);
    QDir targetDir(targetPath);
    targetDir.removeRecursively();
}

void WrapperScripts::deleteScripts()
{
    QString targetPath = ::targetConfigurationPath();
    QDir targetDir(targetPath);
    targetDir.removeRecursively();
}

bool WrapperScripts::checkScripts(const QString &group, const QString &name)
{
    // Find out if everything is there
    QString targetPath = targetConfigurationPath(group, name);
    QDir targetDir(targetPath + QDir::separator() + QStringLiteral("bin"));
    for (size_t i = 0; i < sizeof(wrapperScripts) / sizeof(wrapperScripts[0]); ++i) {
        if (!targetDir.exists(QLatin1String(wrapperScripts[i]))) {
            return false;
        }
    }

    return true;
}
