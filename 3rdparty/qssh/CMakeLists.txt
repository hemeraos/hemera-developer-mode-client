project(hemera-dev-controller CXX)

cmake_minimum_required(VERSION 2.8.10)

find_package(Qt5 COMPONENTS Core Network Widgets Concurrent REQUIRED)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DQT_NO_CAST_TO_ASCII -DQT_DISABLE_DEPRECATED_BEFORE=0x040900)
if (NOT APPLE)
    add_definitions(-DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION)
endif ()

add_subdirectory(src)
