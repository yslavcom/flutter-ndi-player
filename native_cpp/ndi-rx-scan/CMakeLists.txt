cmake_minimum_required(VERSION 3.18.1)

project(
  ndi-monitor
  VERSION 1.0
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME ndi_rx_scan)

add_compile_options(-fPIC)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.[h,c]pp")

add_library(${TARGET_NAME} STATIC ${SOURCES} ${LIB_NDI_VAR})

set (NDI_PATH "/home/iaroslav/work/streaming/NDI/Android/NDI\ SDK\ for\ Android/include/")

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${NDI_PATH} ../)
target_link_libraries(${TARGET_NAME} PUBLIC ${LIB_NDI_VAR})
