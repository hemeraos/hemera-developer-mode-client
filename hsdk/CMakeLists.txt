add_definitions(-DHSDK_VERSION_MAJOR="${HEMERA_SDK_ABI_VERSION}")
add_definitions(-DHSDK_VERSION_MINOR="${HEMERA_SDK_MINOR_VERSION}")
add_definitions(-DHSDK_VERSION_PATCH="${HEMERA_SDK_RELEASE_VERSION}")

set(hemera-hsdk-exec_SRCS
    # Main files
    main.cpp
    hsdkcommand.cpp

    # "Interface" files
    basecommand.cpp
    baseremotecommand.cpp
    remotebuildcommand.cpp

    # Specific commands
    deploycommand.cpp
    devicecommand.cpp
    emulatorcommand.cpp
    hacommand.cpp
    invokecommand.cpp
    launchcommand.cpp
    outputcommand.cpp
    gdbcommand.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/3rdparty/qssh/src/libs/3rdparty/botan)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/qssh/src/libs/ssh)

add_executable(hemera-hsdk-exec ${hemera-hsdk-exec_SRCS})
set_target_properties(hemera-hsdk-exec PROPERTIES
                      OUTPUT_NAME hsdk
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
                      SKIP_BUILD_RPATH FALSE
                      BUILD_WITH_INSTALL_RPATH TRUE
                      INSTALL_RPATH_USE_LINK_PATH TRUE
                      INSTALL_RPATH $ORIGIN/../lib/qtcreator)

qt5_use_modules(hemera-hsdk-exec Core Network)

target_link_libraries(hemera-hsdk-exec HemeraDeveloperModeClient)

install(TARGETS hemera-hsdk-exec DESTINATION bin COMPONENT HemeraSDKHSdkComponents)
