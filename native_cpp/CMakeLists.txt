cmake_minimum_required(VERSION 3.25.1)
project(video-app C CXX)
set(CMAKE_CXX_STANDARD 20)
set(TARGET_NAME ndi-monitor)

include_directories(${GLIB_INCLUDE_DIRS})

add_subdirectory(ndi-rx)
