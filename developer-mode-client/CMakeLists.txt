set(VERSION_MAJOR "0" CACHE INTEGER "Application major version")
set(VERSION_MINOR "2" CACHE INTEGER "Application minor version")
set(VERSION_PATCH "90" CACHE INTEGER "Application patch version")

add_definitions(-DVERSION_MAJOR="${VERSION_MAJOR}")
add_definitions(-DVERSION_MINOR="${VERSION_MINOR}")
add_definitions(-DVERSION_PATCH="${VERSION_PATCH}")

set(dev-controller_SRCS
    configuredevices.cpp
    configuretarget.cpp
    main.cpp
    mainwindow.cpp
    starinfowidget.cpp
)

set(dev-controller_HDRS
    configuredevices.h
    configuretarget.h
    mainwindow.h
)

set(dev-controller_UI
    ../ui/configuredevices.ui
    ../ui/configuretarget.ui
    ../ui/mainwindow.ui
    ../ui/starinfowidget.ui
)

set(image_RCCS ../images/images.qrc)

qt5_add_resources(RESOURCES ${image_RCCS})
qt5_wrap_ui(dev-controller_UI_H ${dev-controller_UI})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/lib)

if (APPLE)
    # Define some settings for the Bundle
    set( MACOSX_BUNDLE_BUNDLE_NAME "Hemera Developer Mode Client" )
    set( MACOSX_BUNDLE_GUI_IDENTIFIER "com.ispirata.Hemera.DeveloperMode.Client" )
    set( MACOSX_BUNDLE_ICON_FILE Hemera.icns )
    set( MACOSX_BUNDLE_INFO_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}, Copyright 2013 Ispirata S.r.l." )
    set( MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}" )
    set( MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
    set( MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
    set( MACOSX_BUNDLE_COPYRIGHT "(C) 2013 Ispirata S.r.l." )

    add_executable(dev-controller MACOSX_BUNDLE ${dev-controller_SRCS} ${dev-controller_UI_H} ${RESOURCES})

    # Allows for bundle re-creation just by running "make". Also installs bundle icon
    add_custom_target( osx_bundle_dirs
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Resources
        COMMAND cp ${CMAKE_SOURCE_DIR}/images/${MACOSX_BUNDLE_ICON_FILE}
                   ${CMAKE_CURRENT_BINARY_DIR}/${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Resources/${MACOSX_BUNDLE_ICON_FILE}
        # Qt translations - uncomment this line when you need to install them to the bundle
        #COMMAND cp *.qm ${CMAKE_CURRENT_BINARY_DIR}/${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Resources/
    )

    add_dependencies( dev-controller osx_bundle_dirs )

    # This tells cmake where to place files inside the bundle
    #set_source_files_properties( ${ProjectName_RESOURCES} ${ProjectName_TRANSLATIONS}
    #                             PROPERTIES MACOSX_PACKAGE_LOCATION Resources )

    set_target_properties(dev-controller PROPERTIES
                                         OUTPUT_NAME "Hemera Developer Mode Client")
else ()
    add_executable(dev-controller ${dev-controller_SRCS} ${dev-controller_UI_H} ${RESOURCES})
    if (UNIX)
        set_target_properties(dev-controller PROPERTIES
                                             OUTPUT_NAME hemera-developer-mode-client
                                             RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
                                             SKIP_BUILD_RPATH FALSE
                                             BUILD_WITH_INSTALL_RPATH TRUE
                                             INSTALL_RPATH_USE_LINK_PATH TRUE
                                             INSTALL_RPATH $ORIGIN/../lib)
    else ()
        set_target_properties(dev-controller PROPERTIES
                                             OUTPUT_NAME HemeraDevModeClient)
    endif ()
endif ()

qt5_use_modules(dev-controller Core Network Widgets)

target_link_libraries(dev-controller HemeraDeveloperModeClient)
