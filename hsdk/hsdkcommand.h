#ifndef HSDKCOMMAND_H
#define HSDKCOMMAND_H

#include "basecommand.h"

#include <hemeradevelopermodecontroller.h>

#include "helpermacros.h"

class HsdkCommand : public BaseCommand
{
    Q_OBJECT

public:
    HsdkCommand(const QStringList &arguments, QObject *parent = 0);

    virtual QString name();
    virtual QString briefDescription();
    virtual QString longDescription();

public Q_SLOTS:
    void start();

protected:
    virtual void setupParser(QCommandLineParser *parser, const QStringList &arguments) Q_DECL_OVERRIDE;
    virtual bool parseAndExecute(QCommandLineParser *parser, const QStringList &arguments) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void onFinished(int exitStatus);

private:
    QStringList m_arguments;
    QCommandLineOption m_dumpOption;
};

// Simple commands
DECLARE_SIMPLE_REMOTE_COMMAND(WipeCommand, "wipe", QT_TRANSLATE_NOOP("SimpleCommands", "Wipe all build files from a Hemera target."),
                              QT_TRANSLATE_NOOP("SimpleCommands", "Wipe all build files from a Hemera target.\n"
                                                                  "The target must be a Hemera SDK VM for this command to work."), "hemera-wipe", QStringList())
DECLARE_SIMPLE_REMOTE_COMMAND(ConfigureCommand, "configure", QT_TRANSLATE_NOOP("SimpleCommands", "Generates support files for a Hemera project."),
                              QT_TRANSLATE_NOOP("SimpleCommands", "Generates support files for a Hemera project."), "hemera-configure",
                              QStringList() << Hemera::DeveloperMode::Controller::localPathToVm())

// Simple build commands
DECLARE_SIMPLE_REMOTE_BUILD_COMMAND(CmakeCommand, "cmake", QT_TRANSLATE_NOOP("SimpleCommands", "Call 'cmake' on a Hemera project."),
                                    QT_TRANSLATE_NOOP("SimpleCommands", "Call 'cmake' on a Hemera project."), "hemera-cmake")
DECLARE_SIMPLE_REMOTE_BUILD_COMMAND(FullBuildCommand, "full-build", QT_TRANSLATE_NOOP("SimpleCommands", "Performs a full build on a Hemera project."),
                                    QT_TRANSLATE_NOOP("SimpleCommands", "Performs a full build on a Hemera project.\n"
                                                                        "It performs these steps in the following order:\n"
                                                                        "cmake, make, make install, rpmbuild."), "hemera-full-build")
DECLARE_SIMPLE_REMOTE_BUILD_COMMAND(BuildProjectCommand, "build-project", QT_TRANSLATE_NOOP("SimpleCommands",
                                                                                            "Performs a full build on a Hemera project without packaging it."),
                                    QT_TRANSLATE_NOOP("SimpleCommands", "Performs a full build on a Hemera project.\n"
                                                                        "It performs these steps in the following order:\n"
                                                                        "cmake, make, make install."), "hemera-build-project")
DECLARE_SIMPLE_REMOTE_BUILD_COMMAND(BuildPackageCommand, "build-package", QT_TRANSLATE_NOOP("SimpleCommands", "Creates a package out of an already built Hemera project."),
                                    QT_TRANSLATE_NOOP("SimpleCommands", "Creates a package out of an already built Hemera project.\n"
                                                                        "Triggering this action on a project may not cause it to build.\n"), "hemera-rpmbuild")

#endif // HSDKCOMMAND_H
