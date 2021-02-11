set(dev-mode-lib_SRCS
    hemeradevelopermodeapplicationoutput.cpp
    hemeradevelopermodecontroller.cpp
    hemeradevelopermodedevice.cpp
    hemeradevelopermodedeployoperation.cpp
    hemeradevelopermodeemulator.cpp
    hemeradevelopermodeglobalobjects.cpp
    hemeradevelopermodehamanager.cpp
    hemeradevelopermodehyperspacestream.cpp
    hemeradevelopermodestar.cpp
    hemeradevelopermodeshelloperation.cpp
    hemeradevelopermodetarget.cpp
    hemeradevelopermodetargetmanager.cpp

    hemeradevelopermodeoperation.cpp

    hemeradevelopermodehyperdiscoveryclient.cpp

    transports/hemeradevelopermodetransport.cpp
    transports/hemeradevelopermodetcptransport.cpp

    # Private code/classes
    virtualboxcommands.cpp
    wrapperscripts.cpp

    # http server
    HttpServer.cpp
    HttpServerConnection.cpp
    HttpTCPServer.cpp
    http_parser.c

    # There's a price we have to pay for playing the game.
    ${CMAKE_SOURCE_DIR}/3rdparty/hemera-qt5-sdk/plugins/qml/settings/hemeraqmlsandbox.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/hemera-qt5-sdk/plugins/qml/settings/hemeraqmlapplication.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/hemera-qt5-sdk/plugins/qml/settings/hemeraqmlsimplecppapplication.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/hemera-qt5-sdk/plugins/qml/settings/hemeraqmlsimpleqmlapplication.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/hemera-qt5-sdk/plugins/qml/settings/hemeraqmlsimpleqtquick1application.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/hemera-qt5-sdk/plugins/qml/settings/hemeraqmlpythonapplication.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/hemera-qt5-sdk/plugins/qml/settings/hemeraqmlproxiedapplication.cpp
)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/3rdparty/qtjsonstream/src/jsonstream)

# final lib
add_library(HemeraDeveloperModeClient SHARED ${dev-mode-lib_SRCS})
set_target_properties(HemeraDeveloperModeClient PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
                      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

generate_export_header(HemeraDeveloperModeClient
             BASE_NAME HemeraDeveloperModeClient
             EXPORT_MACRO_NAME HemeraDeveloperModeClient_EXPORT
             EXPORT_FILE_NAME hemeradevelopermodeexport.h
             STATIC_DEFINE MyLibrary_BUILT_AS_STATIC
)

target_link_libraries(HemeraDeveloperModeClient Qt5::Core Qt5::Network Qt5::Qml bzip2_hemera_static QtJsonStream)

install(TARGETS HemeraDeveloperModeClient DESTINATION lib/qtcreator COMPONENT HemeraSDKCommonComponents)
if (WIN32)
# Also in bin
install(TARGETS HemeraDeveloperModeClient DESTINATION bin COMPONENT HemeraSDKCommonComponents)
endif()
