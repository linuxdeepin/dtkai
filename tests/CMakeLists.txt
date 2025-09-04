# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)

# Enable testing
enable_testing()

# Add debug definitions for stub
add_definitions(-DDEBUG_STUB_INVOKE)

# Add unit tests
add_subdirectory(unit)
