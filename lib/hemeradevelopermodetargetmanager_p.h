/*
 *
 */

#ifndef HEMERA_DEVELOPERMODE_TARGETMANAGER_P_H
#define HEMERA_DEVELOPERMODE_TARGETMANAGER_P_H

#include "hemeradevelopermodetargetmanager.h"

#include "hemeradevelopermodeoperation.h"

#include "hemeradevelopermodeexport.h"

#include <QtCore/QUrl>

/* Place a 1 beside your platform, and 0 elsewhere.
 Generic 32-bit Unix.
 Also works on 64-bit Unix boxes.
 This is the default.
 */
#define BZ_UNIX      1

/*--
 Win32, as seen by Jacob Navia's excellent
 port of (Chris Fraser & David Hanson)'s excellent
 lcc compiler.  Or with MS Visual C.
 This is selected automatically if compiled by a compiler which
 defines _WIN32, not including the Cygwin GCC.
 --*/
#define BZ_LCCWIN32  0

#if defined(_WIN32) && !defined(__CYGWIN__)
#undef  BZ_LCCWIN32
#define BZ_LCCWIN32 1
#undef  BZ_UNIX
#define BZ_UNIX 0
#endif

#define ERROR_IF_EOF(i)       { if ((i) == EOF)  setFinishedWithError(QStringLiteral("org.bzip2.IOERROR"), tr("I/O or other error, bailing out.")); }
#define ERROR_IF_NOT_ZERO(i)  { if ((i) != 0)    setFinishedWithError(QStringLiteral("org.bzip2.IOERROR"), tr("I/O or other error, bailing out.")); }
#define ERROR_IF_MINUS_ONE(i) { if ((i) == (-1)) setFinishedWithError(QStringLiteral("org.bzip2.IOERROR"), tr("I/O or other error, bailing out.")); }

/*---------------------------------------------*/
/*--Platform-specific stu*ff.
 --*/

#if BZ_UNIX
#   include <fcntl.h>
#   include <sys/types.h>
#   include <utime.h>
#   include <unistd.h>
#   include <sys/stat.h>
#   include <sys/times.h>

#   define PATH_SEP    '/'
#   define MY_LSTAT    lstat
#   define MY_STAT     stat
#   define MY_S_ISREG  S_ISREG
#   define MY_S_ISDIR  S_ISDIR

#   define APPEND_FILESPEC(root, name) \
root=snocString((root), (name))
#   define APPEND_FLAG(root, name) \
root=snocString((root), (name))
#   define SET_BINARY_MODE(fd) /**/
#   ifdef __GNUC__
#      define NORETURN __attribute__ ((noreturn))
#   else
#      define NORETURN /**/
#   endif

#   ifdef __DJGPP__
#     include <io.h>
#     include <fcntl.h>
#     undef MY_LSTAT
#     undef MY_STAT
#     define MY_LSTAT stat
#     define MY_STAT stat
#     undef SET_BINARY_MODE
#     define SET_BINARY_MODE(fd)                        \
do {                                            \
    int retVal = setmode ( fileno ( fd ),        \
    O_BINARY );           \
    ERROR_IF_MINUS_ONE ( retVal );               \
    } while ( 0 )
#   endif

#   ifdef __CYGWIN__
#     include <io.h>
#     include <fcntl.h>
#     undef SET_BINARY_MODE
#     define SET_BINARY_MODE(fd)                        \
do {                                            \
int retVal = setmode ( fileno ( fd ),        \
O_BINARY );           \
ERROR_IF_MINUS_ONE ( retVal );               \
} while ( 0 )
#   endif
#endif /* BZ_UNIX */



#if BZ_LCCWIN32
#   include <io.h>
#   include <fcntl.h>
#   include <sys\stat.h>

#   define NORETURN       /**/
#   define PATH_SEP       '\\'
#   define MY_LSTAT       _stat
#   define MY_STAT        _stat
#   define MY_S_ISREG(x)  ((x) & _S_IFREG)
#   define MY_S_ISDIR(x)  ((x) & _S_IFDIR)

#   define APPEND_FLAG(root, name) \
root=snocString((root), (name))

#   define APPEND_FILESPEC(root, name)                \
root = snocString ((root), (name))

#   define SET_BINARY_MODE(fd)                        \
do {                                            \
    int retVal = setmode ( fileno ( fd ),        \
    O_BINARY );           \
    ERROR_IF_MINUS_ONE ( retVal );               \
    } while ( 0 )

#endif /* BZ_LCCWIN32 */

namespace Hemera {
namespace DeveloperMode {

class HemeraDeveloperModeClient_EXPORT InstallEmulatorOperation : public Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(InstallEmulatorOperation)

public:
    explicit InstallEmulatorOperation(const QString &name, const QString &filePath, TargetManager::EmulatorInstallModes mode,
                                      const QString &pathToHsdk, QObject* parent = Q_NULLPTR);
    explicit InstallEmulatorOperation(const QString &name, const QString &token, const QUrl &server, const QString &pathToHsdk,
                                      TargetManager::EmulatorInstallModes mode, QObject* parent = Q_NULLPTR);
    virtual ~InstallEmulatorOperation();

protected:
    virtual void startImpl() override;

    void setThingsToDo(int things);
    void oneThingLessToDo();

private:
    QString m_name;
    TargetManager::EmulatorInstallModes m_mode;
    QString m_hsdk;

    QString m_filePath;
    QString m_token;
    QUrl m_server;

    int m_totalThings;
    int m_leftThings;
};


class HemeraDeveloperModeClient_EXPORT RemoveEmulatorOperation : public Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(RemoveEmulatorOperation)

public:
    explicit RemoveEmulatorOperation(const QString &name, bool keepFiles, QObject* parent = Q_NULLPTR);
    virtual ~RemoveEmulatorOperation();

protected:
    virtual void startImpl() override;

private:
    QString m_name;
    bool m_keepFiles;
};


class TargetManagerPrivate
{
public:
    TargetManagerPrivate(TargetManager *q) : q(q) {}

    TargetManager * const q;

    QHash< QString, Device::Ptr > devicesPool;
    QHash< QString, Emulator::Ptr > emulatorsPool;

    Emulator::Ptr emulatorFromCache(const QString &id);
    Device::Ptr deviceFromCache(const QString &name);

    // For our classes
    Device::Ptr fromRawPointer(Device *device) const;
    Device::ConstPtr fromRawPointer(const Device *device) const;
    Emulator::Ptr fromRawPointer(Emulator *emulator) const;
    Emulator::ConstPtr fromRawPointer(const Emulator *emulator) const;
    Target::Ptr fromRawPointer(Target *target) const;
    Target::ConstPtr fromRawPointer(const Target *target) const;
};

}
}

#endif // HEMERA_DEVELOPERMODE_TARGETMANAGER_H
