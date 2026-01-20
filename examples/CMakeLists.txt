# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)

project(dtkai_examples VERSION 1.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
add_subdirectory(nlp)
add_subdirectory(vision)
add_subdirectory(speech)
add_subdirectory(other)

add_executable(
    presentation_demo
    presentation_demo.cpp
)

target_link_libraries(
    presentation_demo PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Multimedia
    ${DtkCore_LIBRARIES}
    dtkai
)
endif()

add_executable(
    dtkaivalidator
    dtkaivalidator.cpp
)

target_link_libraries(
    dtkaivalidator PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Multimedia
    ${DtkCore_LIBRARIES}
    dtkai
)

# Install dtkaivalidator executable
install(TARGETS dtkaivalidator
    RUNTIME DESTINATION bin
    COMPONENT examples
)
