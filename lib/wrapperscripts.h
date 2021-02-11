#ifndef WRAPPERSCRIPTS_H
#define WRAPPERSCRIPTS_H

#include <QtCore/QString>

#include <hemeradevelopermodetarget.h>

class WrapperScripts
{
public:
    static bool createScripts(const QString &group, const QString &name, const QString &pathToHsdk);
    static void deleteScripts(const QString &group, const QString &name);
    static void deleteScripts();

    static QString targetConfigurationPath(const QString &group, const QString &name);

    static bool checkScripts(const QString &group, const QString &name);

private:
    static bool createScript(const QString &group, const QString &name, const QString &pathToHsdk, int scriptIndex);
};

#endif // WRAPPERSCRIPTS_H
