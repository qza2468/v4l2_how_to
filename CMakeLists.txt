cmake_minimum_required(VERSION 3.21)
project(untitled2 C)

set(CMAKE_C_STANDARD 99)

add_executable(pic_capture v4l2图片采集/picture_capture.c opencv视频采集/video_capture.cpp)
