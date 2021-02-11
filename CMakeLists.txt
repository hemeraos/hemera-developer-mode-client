project(hemera-dev-controller C CXX)

cmake_minimum_required(VERSION 2.8.10)

# Policies
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0043 NEW)
cmake_policy(SET CMP0020 OLD)

find_package(Qt5 COMPONENTS Core Network Widgets Concurrent Qml REQUIRED)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(HEMERA_SDK_ABI_VERSION 0)
set(HEMERA_SDK_MINOR_VERSION 3)
set(HEMERA_SDK_RELEASE_VERSION 0)
set(HEMERA_SDK_VERSION ${HEMERA_SDK_ABI_VERSION}.${HEMERA_SDK_MINOR_VERSION}.${HEMERA_SDK_RELEASE_VERSION})

include(GenerateExportHeader)

set(IFW_PATH ${CMAKE_SOURCE_DIR}/../ifw CACHE PATH "Qt Installer Framework location")
set(MAKE_EXECUTABLE "make" CACHE PATH "Make executable")

file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR}/install FINAL_INSTALL_DIR)

# Always use RPath. We're gonna throw some packages in this shit.
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Self explanatory...
SET(CMAKE_INSTALL_RPATH "$ORIGIN/../../../../../../anrpathlongenoughsoitcanactuallybereplacedatsomepoint")

# Compile C files first.
add_subdirectory(3rdparty/bzip2)

# C++11
if (CMAKE_COMPILER_IS_GNUCXX)
   execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
   if (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
        message(STATUS "C++11 activated.")
        add_definitions("-std=gnu++11")
   elseif (GCC_VERSION VERSION_GREATER 4.3 OR GCC_VERSION VERSION_EQUAL 4.3)
        message(WARNING "C++0x activated. If you get any errors update to a compiler which fully supports C++11")
        add_definitions("-std=gnu++0x")
   else ()
        message(FATAL_ERROR "C++11 needed. Therefore a gcc compiler with a version higher than 4.3 is needed.")
   endif()
else (CMAKE_COMPILER_IS_GNUCXX)
   add_definitions("-std=c++0x -stdlib=libc++")
endif (CMAKE_COMPILER_IS_GNUCXX)

# Only generating headers in 3rd parties
set(GENERATE_HEADERS_ONLY ON)
add_subdirectory(3rdparty)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/lib ${CMAKE_BINARY_DIR}/lib ${CMAKE_SOURCE_DIR}/3rdparty)

add_subdirectory(lib)
add_subdirectory(developer-mode-client)
add_subdirectory(hsdk)
add_subdirectory(translations)

get_property(QMAKE_LOCATION TARGET Qt5::qmake PROPERTY LOCATION)

################
################ INSTALLER FRAMEWORK PACKAGES
################

set(CPACK_PACKAGE_NAME "HemeraSDK")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Hemera SDK Installer")
set(CPACK_PACKAGE_VERSION "1.0.0") # Version of installer
set(CPACK_PACKAGE_VENDOR "Ispirata")
set(CPACK_DOWNLOAD_ALL ON)
set(CPACK_GENERATOR "IFW")
set(CPACK_IFW_PRODUCT_URL "http://hemera.io/")
set(CPACK_IFW_PACKAGE_WINDOW_ICON ${CMAKE_SOURCE_DIR}/images/hemera.png)
#set(CPACK_IFW_PACKAGE_LOGO ${CMAKE_SOURCE_DIR}/images/hemera.png)
set(CPACK_IFW_TARGET_DIRECTORY "@ApplicationsDir@/HemeraSDK")

include(CPack)
include(CPackIFW)

cpack_add_component(HemeraSDKCommonComponents
    DISPLAY_NAME "Common files"
    DESCRIPTION "Hemera SDK files needed by all components."
    REQUIRED
    DOWNLOADED)
cpack_ifw_configure_component(HemeraSDKCommonComponents
    VERSION "${HEMERA_SDK_VERSION}" # Version of component
    NAME "io.hemera.HemeraSDK.Common")

cpack_add_component(HemeraSDKHSdkComponents
    DISPLAY_NAME "Command line tools"
    DESCRIPTION "Hemera SDK tools to be used from command line. Needed for basic functionalities."
    REQUIRED
    DOWNLOADED)
cpack_ifw_configure_component(HemeraSDKHSdkComponents
    VERSION "${HEMERA_SDK_VERSION}" # Version of component
    NAME "io.hemera.HemeraSDK.HSdk"
    DEPENDS "io.hemera.HemeraSDK.Common")

cpack_add_component(HemeraSDKHeadersComponents
    DISPLAY_NAME "Hemera SDK Headers"
    DESCRIPTION "Development headers for Hemera SDK"
    DOWNLOADED)
cpack_ifw_configure_component(HemeraSDKHeadersComponents
    VERSION "${HEMERA_SDK_VERSION}" # Version of component
    NAME "io.hemera.HemeraSDK.Headers")

# We need those because of the damn 3rdparty installation...
cpack_add_component(HemeraQt5SDK
    DISPLAY_NAME "Hemera Qt5 SDK Runtime Components"
    DESCRIPTION "Hemera Qt5 SDK client runtime files, needed to parse and detect descriptors."
    REQUIRED
    DOWNLOADED)
cpack_ifw_configure_component(HemeraQt5SDK
    VERSION "${HEMERA_SDK_VERSION}" # Version of component
    NAME "io.hemera.HemeraSDK.HemeraQt5SDK"
    DEPENDS "io.hemera.HemeraSDK.Common")

if (WIN32)
    set(HEMERA_SDK_DOWNLOAD_REPOSITORY "http://hemera.io/downloads/sdk/repository/Windows/")
elseif (APPLE)
    set(HEMERA_SDK_DOWNLOAD_REPOSITORY "http://hemera.io/downloads/sdk/repository/OSX/")
else ()
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(HEMERA_SDK_DOWNLOAD_REPOSITORY "http://hemera.io/downloads/sdk/repository/Linux64/")
else ()
    set(HEMERA_SDK_DOWNLOAD_REPOSITORY "http://hemera.io/downloads/sdk/repository/Linux32/")
endif ()
endif ()

cpack_ifw_add_repository(myrepo
    URL ${HEMERA_SDK_DOWNLOAD_REPOSITORY}
    DISPLAY_NAME "Official Hemera installation repositories")
cpack_configure_downloads(${HEMERA_SDK_DOWNLOAD_REPOSITORY}
                          ${CMAKE_BINARY_DIR}/repositories
                          ALL)
