# TODO: Remove this entirely after we got the hang of the non-SSH world
#add_subdirectory(qssh)
# We need to compile settings plugin.
add_subdirectory(hemera-qt5-sdk/plugins/qml/settings)
add_subdirectory(hemera-qt5-sdk/src)

########## HERE BE DRAGONS, LOGIC FOR QTJSONSTREAM ####################
add_definitions(-DQT_ADDON_JSONSTREAM_LIB)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/qtjsonstream/src/jsonstream
    ${CMAKE_CURRENT_SOURCE_DIR}/qtjsonstream/src/jsonstream/bson
    ${CMAKE_CURRENT_SOURCE_DIR}/qtjsonstream/src/jsonstream/qtjsonschema
)

set(qtjsonstream_SRCS
    qtjsonstream/src/jsonstream/qjsonstream.cpp
    qtjsonstream/src/jsonstream/qjsonbuffer.cpp
    qtjsonstream/src/jsonstream/bson/bson.cpp
    qtjsonstream/src/jsonstream/bson/qt-bson.cpp
    qtjsonstream/src/jsonstream/qjsonclient.cpp
    qtjsonstream/src/jsonstream/qjsonauthority.cpp
    qtjsonstream/src/jsonstream/qjsontokenauthority.cpp
    qtjsonstream/src/jsonstream/qjsonserverclient.cpp
    qtjsonstream/src/jsonstream/qjsonserver.cpp
    qtjsonstream/src/jsonstream/qjsonpipe.cpp
    qtjsonstream/src/jsonstream/qjsonconnection.cpp
    qtjsonstream/src/jsonstream/qjsonconnectionprocessor.cpp
    qtjsonstream/src/jsonstream/qjsonendpoint.cpp
    qtjsonstream/src/jsonstream/qjsonendpointmanager.cpp
    qtjsonstream/src/jsonstream/qtjsonschema/qjsonschemaerror.cpp
    qtjsonstream/src/jsonstream/qtjsonschema/qjsonschemavalidator.cpp
)

if (UNIX AND NOT APPLE)
    set(qtjsonstream_SRCS ${qtjsonstream_SRCS}
        qtjsonstream/src/jsonstream/qjsonpidauthority.cpp
        qtjsonstream/src/jsonstream/qjsonuidauthority.cpp
        qtjsonstream/src/jsonstream/qjsonuidrangeauthority.cpp
        )
endif ()

add_library(QtJsonStream SHARED ${qtjsonstream_SRCS})
set_target_properties(QtJsonStream PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
                      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

target_link_libraries(QtJsonStream Qt5::Core Qt5::Network Qt5::Qml)

install(TARGETS QtJsonStream DESTINATION lib/qtcreator COMPONENT HemeraSDKCommonComponents)
if (WIN32)
# Also in bin
install(TARGETS QtJsonStream DESTINATION bin COMPONENT HemeraSDKCommonComponents)
endif()
