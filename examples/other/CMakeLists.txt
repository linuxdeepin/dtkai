cmake_minimum_required(VERSION 3.13)

project(other_examples VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Model information test
add_executable(
    model_info_test
    model_info_test.cpp
)

target_link_libraries(
    model_info_test PRIVATE
    ${DtkCore_LIBRARIES}
    dtkai
)
