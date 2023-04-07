cmake_minimum_required(VERSION 3.18.1)

project(
  ndi-monitor
  VERSION 1.0
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME player)

add_compile_options(-fPIC)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.[h,c]pp")

# GLESv2
find_path(GLES3_INCLUDE_DIR GLES3/gl3.h HINTS ${ANDROID_NDK})
find_library(GLES3_LIBRARY libGLESv3.so HINTS ${GLES3_INCLUDE_DIR}/../lib)


add_library(${TARGET_NAME} STATIC ${SOURCES} ${LIB_NDI_VAR})

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${GLES3_INCLUDE_DIR} ../)
target_link_libraries(${TARGET_NAME} PUBLIC ${LIB_NDI_VAR} ${GLES3_LIBRARY})
