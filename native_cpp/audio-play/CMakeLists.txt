cmake_minimum_required(VERSION 3.16.3)

# project(
#   ndi-monitor
#   VERSION 1.0
#   LANGUAGES CXX)
project(CMakeRustSample)

set(TARGET_NAME audio)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pthread")

enable_language(Rust)
include(CMakeCargo)

include_directories("include")
add_subdirectory(audio-play-lib)

add_compile_options(-fPIC)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.[h,c]pp")
add_library(${TARGET_NAME} STATIC ${SOURCES})
# target_link_libraries(${TARGET_NAME} PUBLIC)

#add_executable(${TARGET_NAME} ${SOURCES})
target_link_libraries(${TARGET_NAME} audio-play-lib dl pthread)