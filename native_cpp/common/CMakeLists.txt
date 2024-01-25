# cmake_minimum_required(VERSION 3.18.1)
cmake_minimum_required(VERSION 3.16.3)

project(
  ndi-monitor
  VERSION 1.0
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME common)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g0")

add_compile_options(-fPIC)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.[h,c]pp")
add_library(${TARGET_NAME} STATIC ${SOURCES})

if(ANDROID)
    add_definitions(-DANDROID_PLATFORM)  # Define a preprocessor macro
elseif(UNIX AND NOT APPLE)
    add_definitions(-DLINUX_PLATFORM)
endif()


if (ANDROID)
    find_library(log-lib log)
    target_link_libraries(${TARGET_NAME} PUBLIC  ${log-lib}  )
endif ()

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ../)
