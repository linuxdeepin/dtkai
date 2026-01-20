# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ut-dtkai)

# Test-specific compilation options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-access-control -fno-inline -Wno-pmf-conversions")

file(GLOB_RECURSE INCLUDE_FILES "${PROJECT_SOURCE_DIR}/include/dtkai/*")
file(GLOB_RECURSE SRCS
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Collect test files
file(GLOB_RECURSE TEST_FILES 
    "./*.h" 
    "./*.cpp" 
    "${PROJECT_SOURCE_DIR}/tests/3rdparty/stub-ext/*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/3rdparty/cpp-stub/*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/common/*.cpp"
)

# Create test executable
add_executable(${BIN_NAME} ${TEST_FILES} test_resources.qrc)

# Include directories
target_include_directories(${BIN_NAME} PRIVATE
    # Project header directories
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/dtkai
    
    # Source code directory
    ${PROJECT_SOURCE_DIR}/src
    
    # Build directory (for DBus generated headers)
    ${PROJECT_BINARY_DIR}/src
    
    # Test framework directories
    ${PROJECT_SOURCE_DIR}/tests/3rdparty/cpp-stub
    ${PROJECT_SOURCE_DIR}/tests/3rdparty/stub-ext
    ${PROJECT_SOURCE_DIR}/tests/common
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Test
    Dtk${DTK_VERSION_MAJOR}::Core
    dtkai
    ${CMAKE_DL_LIBS}
    pthread
    gcov
    gtest
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${BIN_NAME} PRIVATE --coverage)
    target_link_options(${BIN_NAME} PRIVATE --coverage)
endif()

add_test(NAME ${BIN_NAME} COMMAND ${BIN_NAME})
