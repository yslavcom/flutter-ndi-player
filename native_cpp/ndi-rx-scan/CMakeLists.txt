# cmake_minimum_required(VERSION 3.18.1)
cmake_minimum_required(VERSION 3.16.3)

project(
  ndi-monitor
  VERSION 1.0
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME ndi_rx_scan)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g0")

add_compile_options(-fPIC)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.[h,c]pp")

add_library(${TARGET_NAME} STATIC ${SOURCES} ${LIB_NDI_VAR})

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${NDI_PATH} ../)
target_link_libraries(${TARGET_NAME} PUBLIC ${LIB_NDI_VAR})
