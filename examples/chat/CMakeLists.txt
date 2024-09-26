cmake_minimum_required(VERSION 3.13)

project(chat VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
find_package(Dtk COMPONENTS Core REQUIRED)
#find_package(dtkai REQUIRED)

add_executable(
    chat
    chat.cpp
)

add_executable(
    chat_stream
    chat_stream.cpp
)

add_executable(
    functioncalling
    functioncalling.cpp
)

target_link_libraries(
    chat PRIVATE
    ${DtkCore_LIBRARIES}
    dtkai
)

target_link_libraries(
    chat_stream PRIVATE
    ${DtkCore_LIBRARIES}
    dtkai
)

target_link_libraries(
    functioncalling PRIVATE
    ${DtkCore_LIBRARIES}
    dtkai
)
