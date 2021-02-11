DEFINES += HEMERA_LIBRARY

# Hemera files

SOURCES += hemeraplugin.cpp \
    hyperspaceprocess.cpp \
    hemerabuildpackagestep.cpp \
    hemeradevicetester.cpp \
    hemeraeditor.cpp \
    hemeraeditorfactory.cpp \
    hemerafilecompletionassist.cpp \
    hemeraopenprojectwizard.cpp \
    hemerahighlighter.cpp \
    hemeraproject.cpp \
    hemeraprojectmanager.cpp \
    hemeraprojectnodes.cpp \
    hemerarunconfiguration.cpp \
    hemeratoolchain.cpp \
    hemerabuildconfiguration.cpp \
    hemeratarget.cpp \
    hemeradevice.cpp \
    hemeratargetfactory.cpp \
    hemeraemulator.cpp \
    hemeradeviceconfigurationwidget.cpp \
    hemeraconfigurationpages.cpp \
    hemerawizards.cpp \
    hemeraqtversion.cpp \
    hemeralistener.cpp \
    hemerasdkmanager.cpp \
    hemerakitinformation.cpp \
    hsdkstep.cpp \
    hsdkvalidator.cpp \
    hemeradeployconfiguration.cpp \
    hemeradeploystep.cpp

HEADERS += hemeraplugin.h \
        hemera_global.h \
        hemeraconstants.h \
    hyperspaceprocess.h \
    hemerabuildpackagestep.h \
    hemeradevicetester.h \
    hemeraeditor.h \
    hemeraeditorfactory.h \
    hemerafilecompletionassist.h \
    hemerahighlighter.h \
    hemeraopenprojectwizard.h \
    hemeraproject.h \
    hemeraprojectmanager.h \
    hemeraprojectnodes.h \
    hemerarunconfiguration.h \
    hemeratoolchain.h \
    hemerabuildconfiguration.h \
    hemerabuildinfo.h \
    hemeratarget.h \
    hemeradevice.h \
    hemeratargetfactory.h \
    hemeraemulator.h \
    hemeradeviceconfigurationwidget.h \
    hemeraconfigurationpages.h \
    hemerawizards.h \
    hemeraqtversion.h \
    hemeralistener.h \
    hemerasdkmanager.h \
    hemerakitinformation.h \
    hsdkstep.h \
    hsdkvalidator.h \
    hemeradeployconfiguration.h \
    hemeradeploystep.h

FORMS += hemeraemulatorsetupwidget.ui \
    hemeradevicesetupwidget.ui \
    hemeradeviceassociatewidget.ui \
    hemeradeviceconfigurationwidget.ui \
    genericprogresswidget.ui \
    hemerarunconfigurationwidget.ui \
    hemeraemulatorconfigurationwidget.ui \
    hemeralicenseform.ui

RESOURCES += \
    hemeraproject.qrc

INCLUDEPATH += $$PWD/../lib
INCLUDEPATH += $$PWD/../build/lib

LIBS += -L$$PWD/../build/lib -lHemeraDeveloperModeClient

# Qt Creator linking

## set the QTC_SOURCE environment variable to override the setting here
QTCREATOR_SOURCES = $$(QTC_SOURCE)
isEmpty(QTCREATOR_SOURCES):QTCREATOR_SOURCES=$$PWD/../3rdparty/qt-creator

## set the QTC_BUILD environment variable to override the setting here
IDE_BUILD_TREE = $$(QTC_BUILD)
isEmpty(IDE_BUILD_TREE):IDE_BUILD_TREE=$$PWD/../build/qt-creator

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
##    "~/Library/Application Support/QtProject/Qt Creator" on Mac
# USE_USER_DESTDIR = yes

###### If the plugin can be depended upon by other plugins, this code needs to be outsourced to
###### <dirname>_dependencies.pri, where <dirname> is the name of the directory containing the
###### plugin's sources.

QTC_PLUGIN_NAME = Hemera
QTC_LIB_DEPENDS += \
    # nothing here at this time

QTC_PLUGIN_DEPENDS += \
    coreplugin \
    debugger \
    texteditor \
    projectexplorer \
    cpptools \
    qtsupport

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

###### End _dependencies.pri contents ######

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)

DISTFILES += \
    wizards/qtquick2application/wizard.json

