cmake_minimum_required(VERSION 3.13)

project(vision_examples VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Image recognition demo
add_executable(
    image_recognition_demo
    image_recognition_demo.cpp
)

target_link_libraries(
    image_recognition_demo PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    ${DtkCore_LIBRARIES}
    dtkai
)

# OCR client test with embedded resources
add_executable(
    ocr_client_test
    ocr_client_test.cpp
    test_resources.qrc
)

target_link_libraries(
    ocr_client_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    ${DtkCore_LIBRARIES}
    dtkai
)
