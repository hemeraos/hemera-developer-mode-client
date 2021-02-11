#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>
#include <QtCore/QSocketNotifier>
#include <QtCore/QVariant>

#include "hsdkcommand.h"

#ifndef _WIN32
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

static int sighupFd[2];
static int sigtermFd[2];

static void hupSignalHandler(int)
{
    char a = 1;
    ::write(sighupFd[0], &a, sizeof(a));
}

static void termSignalHandler(int)
{
    char a = 1;
    ::write(sigtermFd[0], &a, sizeof(a));
}

static int setup_unix_signal_handlers()
{
    struct sigaction hup, term, sigint;

    hup.sa_handler = hupSignalHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags = 0;
    hup.sa_flags |= SA_RESTART;

    if (sigaction(SIGHUP, &hup, 0) > 0) {
        return 1;
    }

    term.sa_handler = termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags |= SA_RESTART;

    if (sigaction(SIGTERM, &term, 0) > 0) {
        return 2;
    }
    // Handle INT and TERM as if they were the same signal (so we interpret CTRL+C as termination and not interruption)
    if (sigaction(SIGINT, &term, 0) > 0) {
        return 2;
    }

    return 0;
}
#else
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

BOOL WINAPI ConsoleHandler(DWORD dwType)
{
    switch(dwType) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        QCoreApplication::instance()->property("__mainHSDKCommand").value<HsdkCommand*>()->onTermRequest();
        return TRUE;
        break;
    default:
        // Pass
        return FALSE;
        break;
    }
}
#endif

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Hemera SDK CLI");
    QCoreApplication::setApplicationVersion(QString("%1.%2.%3").arg(HSDK_VERSION_MAJOR).arg(HSDK_VERSION_MINOR).arg(HSDK_VERSION_PATCH));

    HsdkCommand mainCommand(QCoreApplication::arguments());

#ifndef _WIN32
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sighupFd)) {
        qFatal("Couldn't create HUP socketpair");
    }

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd)) {
        qFatal("Couldn't create TERM socketpair");
    }

    // Signal handling
    QSocketNotifier snHup(sighupFd[1], QSocketNotifier::Read);
    QObject::connect(&snHup, &QSocketNotifier::activated, [&] () {
            // Handle SIGHUP here
            snHup.setEnabled(false);
            char tmp;
            ::read(sighupFd[1], &tmp, sizeof(tmp));

            // Skip
            return;
    });
    QSocketNotifier snTerm(sigtermFd[1], QSocketNotifier::Read);
    QObject::connect(&snTerm, &QSocketNotifier::activated, [&] () {
            // Handle SIGTERM here
            snTerm.setEnabled(false);
            char tmp;
            ::read(sigtermFd[1], &tmp, sizeof(tmp));

            // Try delegating
            if (!mainCommand.onTermRequest()) {
                QCoreApplication::quit();
            }
            if (mainCommand.signalHandlingBehaviors() & BaseCommand::AllowMultipleSIGINT) {
                snTerm.setEnabled(true);
            }
    });

    if (setup_unix_signal_handlers() != 0) {
        qFatal("Couldn't register UNIX signal handler");
        return -1;
    }
#else
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)) {
        fprintf(stderr, "Unable to install handler!\n");
        return EXIT_FAILURE;
    } else {
        // Now that's a very dirty trick, but it works.
        QCoreApplication::instance()->setProperty("__mainHSDKCommand", QVariant::fromValue(&mainCommand));
    }
#endif

    QMetaObject::invokeMethod(&mainCommand, "start", Qt::QueuedConnection);
    return app.exec();
}
