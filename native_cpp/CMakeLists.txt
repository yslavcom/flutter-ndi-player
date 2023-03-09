cmake_minimum_required(VERSION 3.25.1)
project(video-app C CXX)
set(CMAKE_CXX_STANDARD 20)
set(TARGET_NAME ndi-monitor)

include_directories(${GLIB_INCLUDE_DIRS})

add_subdirectory(ndi-rx)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.[h,c]pp")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_library(${TARGET_NAME} SHARED ${SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ndi-rx)
target_link_libraries(${TARGET_NAME} PUBLIC ndi_rx)
