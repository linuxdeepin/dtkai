# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)

# If do't define version number, specify the version number
set(VERSION "1.0.0" CACHE STRING "define project version")

project(dtkai
    LANGUAGES CXX
    HOMEPAGE_URL https://github.com/linuxdeepin/dtkai
    DESCRIPTION "dtk library for dtkai."
    VERSION ${VERSION}
)

# specify install dir
include(GNUInstallDirs)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra")
set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "-Wl,--as-needed")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

set(CMAKE_CXX_FLAGS Release CACHE STRING "define CXXFLAGS")

find_package(Qt6 COMPONENTS Core QUIET)
if(Qt6_FOUND)
    set(DTK_VERSION_MAJOR 6)
    set(QT_VERSION_MAJOR 6)
else()
    find_package(Qt5 REQUIRED COMPONENTS Core)
    set(QT_VERSION_MAJOR 5)
endif()
message("Using Qt version: ${QT_VERSION_MAJOR}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_EXAMPLES ON)
    set(BUILD_TESTING ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus Test)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED Core)
add_compile_definitions(QT_NO_SIGNALS_SLOTS_KEYWORDS)

# src
add_subdirectory(src)

# example
if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# test
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
