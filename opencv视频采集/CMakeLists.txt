cmake_minimum_required(VERSION 3.22)
project(untitled1)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(untitled1 main.cpp)

target_link_libraries(untitled1 ${OpenCV_LIBS})