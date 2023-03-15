cmake_minimum_required(VERSION 3.25.1)

project(
  ndi-monitor
  VERSION 1.0
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME ndi_app_rx)

add_compile_options(-fPIC)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.[h,c]pp")


add_library(${TARGET_NAME} STATIC ${SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ../)
target_link_libraries(${TARGET_NAME} PUBLIC -lndi /home/iaroslav/work/streaming/NDI/5.5.3/NDI-Advanced-SDK-for-Linux/lib/x86_64-linux-gnu/libndi.so.5.5.3)
