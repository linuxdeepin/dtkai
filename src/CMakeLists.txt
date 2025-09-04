# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME dtkai)

file(GLOB_RECURSE INCLUDE_FILES "${PROJECT_SOURCE_DIR}/include/${BIN_NAME}/*")
file(GLOB_RECURSE SRCS
    "*.h"
    "*.cpp"
    "*/*.h"
    "*/*.cpp"
)

function(add_dbus_interface_by_version xml_file interface_name)
    if(QT_VERSION_MAJOR EQUAL "5")
        qt5_add_dbus_interface(DBUS_FILES ${xml_file} ${interface_name})
    else()
        qt_add_dbus_interface(DBUS_FILES ${xml_file} ${interface_name})
    endif()
    set(DBUS_FILES ${DBUS_FILES} PARENT_SCOPE)
endfunction()

add_dbus_interface_by_version(${CMAKE_SOURCE_DIR}/3rdparty/aidaemon/org.deepin.ai.daemon.sessionmanager.xml aidaemon_sessionmanager)
add_dbus_interface_by_version(${CMAKE_SOURCE_DIR}/3rdparty/aidaemon/org.deepin.ai.daemon.apisession.chat.xml aidaemon_apisession_chat)
add_dbus_interface_by_version(${CMAKE_SOURCE_DIR}/3rdparty/aidaemon/org.deepin.ai.daemon.apisession.functioncalling.xml aidaemon_apisession_functioncalling)
add_dbus_interface_by_version(${CMAKE_SOURCE_DIR}/3rdparty/aidaemon/org.deepin.ai.daemon.apisession.speechtotext.xml aidaemon_apisession_speechtotext)
add_dbus_interface_by_version(${CMAKE_SOURCE_DIR}/3rdparty/aidaemon/org.deepin.ai.daemon.apisession.texttospeech.xml aidaemon_apisession_texttospeech)
add_dbus_interface_by_version(${CMAKE_SOURCE_DIR}/3rdparty/aidaemon/org.deepin.ai.daemon.apisession.imagerecognition.xml aidaemon_apisession_imagerecognition)
add_dbus_interface_by_version(${CMAKE_SOURCE_DIR}/3rdparty/aidaemon/org.deepin.ai.daemon.apisession.ocr.xml aidaemon_apisession_ocr)
add_dbus_interface_by_version(${CMAKE_SOURCE_DIR}/3rdparty/aidaemon/org.deepin.ai.daemon.modelinfo.xml aidaemon_modelinfo)

message(${DBUS_FILES})

add_library(${BIN_NAME} SHARED
    ${INCLUDE_FILES}
    ${SRCS}
    ${DBUS_FILES}
)

target_include_directories(${BIN_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/${BIN_NAME}/)

set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

target_compile_definitions(${BIN_NAME} PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")

find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED Core)
find_package(PkgConfig REQUIRED)
target_include_directories(${BIN_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Core
)

install(FILES ${INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${BIN_NAME})
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

# for pc file config
set(PC_LIBS_PRIVATE Qt${QT_VERSION_MAJOR}Core)
set(PC_REQ_PRIVATE)
set(PC_REQ_PUBLIC)

# config pkgconfig file
configure_file(${PROJECT_SOURCE_DIR}/misc/${BIN_NAME}/${BIN_NAME}.pc.in ${BIN_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# config cmake file
configure_file(${PROJECT_SOURCE_DIR}/misc/${BIN_NAME}/${BIN_NAME}Config.cmake.in ${BIN_NAME}Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${BIN_NAME})

# config qmake moudule file
set(DTK_MODULE ${BIN_NAME})
set(DTK_DEPS "core dbus")
set(QMKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/modules" CACHE STRING "INSTALL DIR FOR qt pri files")
configure_file(${PROJECT_SOURCE_DIR}/misc/${BIN_NAME}/qt_lib_${BIN_NAME}.pri.in qt_lib_${BIN_NAME}.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_${BIN_NAME}.pri DESTINATION "${QMKSPECS_INSTALL_DIR}")
