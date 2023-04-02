cmake_minimum_required(VERSION 3.18.1)

project(
  ndi-monitor
  VERSION 1.0
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME common)

add_compile_options(-fPIC)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.[h,c]pp")
add_library(${TARGET_NAME} STATIC ${SOURCES})

if (ANDROID)
    target_compile_definitions(${TARGET_NAME} PRIVATE ANDROID_OUT=1)
elseif (LINUX)
    target_compile_definitions(${TARGET_NAME} PRIVATE LINUX_OUT=1)
else ()
    target_compile_definitions(${TARGET_NAME} PRIVATE OTHER_OUT=1)
endif ()

if (ANDROID)
    find_library(log-lib log)
    target_link_libraries(${TARGET_NAME} PUBLIC  ${log-lib})
endif ()

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  ../)
