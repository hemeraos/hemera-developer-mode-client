set(Botan_SRCS
    botan.h
    botan.cpp
)

add_definitions(-DBOTAN_DLL=Q_DECL_EXPORT)

include_directories(${Qt5Core_INCLUDE_DIRS})

if ("${CMAKE_SYSTEM}" MATCHES "Linux")
    add_definitions(-DBOTAN_TARGET_OS_IS_LINUX -DBOTAN_TARGET_OS_HAS_CLOCK_GETTIME
                    -DBOTAN_TARGET_OS_HAS_DLOPEN -DBOTAN_TARGET_OS_HAS_GMTIME_R -DBOTAN_TARGET_OS_HAS_POSIX_MLOCK
                    -DBOTAN_HAS_DYNAMICALLY_LOADED_ENGINE -DBOTAN_HAS_DYNAMIC_LOADER)
endif ()
if (UNIX)
    add_definitions(-DBOTAN_TARGET_OS_HAS_GETTIMEOFDAY -DBOTAN_HAS_ALLOC_MMAP
                    -DBOTAN_HAS_ENTROPY_SRC_DEV_RANDOM -DBOTAN_HAS_ENTROPY_SRC_EGD -DBOTAN_HAS_ENTROPY_SRC_FTW
                    -DBOTAN_HAS_ENTROPY_SRC_UNIX -DBOTAN_HAS_MUTEX_PTHREAD -DBOTAN_HAS_PIPE_UNIXFD_IO)
    if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
        set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-fPIC -ansi -fpermissive -finline-functions -Wno-long-long")
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fPIC -ansi -fpermissive -finline-functions -Wno-long-long -fvisibility-inlines-hidden")
    endif ()
endif ()
if (APPLE)
    add_definitions(-DBOTAN_TARGET_OS_IS_DARWIN)
endif ()
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
    add_definitions(-DBOTAN_BUILD_COMPILER_IS_GCC)
else()
    add_definitions(-DBOTAN_BUILD_COMPILER_IS_CLANG)
endif ()

if (WIN32)
    add_definitions(-DBOTAN_TARGET_OS_IS_WINDOWS
                    -DBOTAN_TARGET_OS_HAS_LOADLIBRARY -DBOTAN_TARGET_OS_HAS_WIN32_GET_SYSTEMTIME
                    -DBOTAN_TARGET_OS_HAS_WIN32_VIRTUAL_LOCK -DBOTAN_HAS_DYNAMICALLY_LOADED_ENGINE
                    -DBOTAN_HAS_DYNAMIC_LOADER BOTAN_HAS_ENTROPY_SRC_CAPI -DBOTAN_HAS_ENTROPY_SRC_WIN32
                    -DBOTAN_HAS_MUTEX_WIN32)
endif()

# final lib
add_library(Botan SHARED ${Botan_SRCS})
set_target_properties(Botan PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
                      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if ("${CMAKE_SYSTEM}" MATCHES "Linux")
    target_link_libraries(Botan rt)
endif ()
