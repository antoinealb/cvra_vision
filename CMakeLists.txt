cmake_minimum_required(VERSION 2.8)
project(vision)

find_package(OpenCV REQUIRED)

option(On_Robot "compilation on robot" OFF)

if(On_Robot)
    add_definitions(-DCOMPILE_ON_ROBOT)
endif()

add_library(vision SHARED vision.cpp)

add_executable(main main.cpp)

target_link_libraries(vision ${OpenCV_LIBS})
target_link_libraries(main vision ${OpenCV_LIBS})
