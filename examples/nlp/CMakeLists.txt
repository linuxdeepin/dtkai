cmake_minimum_required(VERSION 3.13)

project(nlp_examples VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Basic chat functionality
add_executable(
    chat
    chat.cpp
)

target_link_libraries(
    chat PRIVATE
    ${DtkCore_LIBRARIES}
    dtkai
)

# Streaming chat functionality
add_executable(
    chat_stream
    chat_stream.cpp
)

target_link_libraries(
    chat_stream PRIVATE
    ${DtkCore_LIBRARIES}
    dtkai
)

# Enhanced chat functionality
add_executable(
    chat_enhanced
    chat_enhanced.cpp
)

target_link_libraries(
    chat_enhanced PRIVATE
    ${DtkCore_LIBRARIES}
    dtkai
)

# Function calling functionality
add_executable(
    functioncalling
    functioncalling.cpp
)

target_link_libraries(
    functioncalling PRIVATE
    ${DtkCore_LIBRARIES}
    dtkai
)
