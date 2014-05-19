cmake_minimum_required(VERSION 2.8)
project(take_pic)

find_package(OpenCV REQUIRED)

add_executable(take_pic take_pic.cpp)

target_link_libraries(take_pic ${OpenCV_LIBS})
