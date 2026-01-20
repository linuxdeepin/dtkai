cmake_minimum_required(VERSION 3.13)

project(speech_examples VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Real-time speech to text
add_executable(
    speechtotext_realtime
    speechtotext_realtime.cpp
)

target_link_libraries(
    speechtotext_realtime PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Multimedia
    ${DtkCore_LIBRARIES}
    dtkai
)

# Text to speech test
add_executable(
    texttospeech_test
    texttospeech_test.cpp
)

target_link_libraries(
    texttospeech_test PRIVATE
    ${DtkCore_LIBRARIES}
    dtkai
)

# Streaming text to speech demo
add_executable(
    texttospeech_streaming_demo
    texttospeech_streaming_demo.cpp
)

target_link_libraries(
    texttospeech_streaming_demo PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Multimedia
    ${DtkCore_LIBRARIES}
    dtkai
)
