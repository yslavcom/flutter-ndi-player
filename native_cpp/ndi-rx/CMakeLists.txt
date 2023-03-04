cmake_minimum_required(VERSION 3.25.1)

project(
  ndi-monitor
  VERSION 1.0
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME ndi_rx)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.[h,c]pp")


add_library(${TARGET_NAME} SHARED ${SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  ${GLIB_LDFLAGS} ../)
