cmake_minimum_required(VERSION 2.8)
project(triangle_detect)

find_package(OpenCV REQUIRED)

add_executable(triangle_detect triangle_detect.cpp)

target_link_libraries(triangle_detect ${OpenCV_LIBS})
