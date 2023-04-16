cmake_minimum_required(VERSION 3.18.1)

project(
  ndi-monitor
  VERSION 1.0
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME player)

add_compile_options(-fPIC)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.[h,c]pp")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# GLESv2
find_path(GLES2_INCLUDE_DIR GLES2/gl2.h HINTS ${ANDROID_NDK})
find_library(GLES2_LIBRARY libGLESv2.so HINTS ${GLES2_INCLUDE_DIR}/../lib)

# EGL
find_path(EGL_INCLUDE_DIR EGL/egl.h HINTS ${ANDROID_NDK})
find_library(EGL_LIBRARY libEGL.so HINTS ${EGL_INCLUDE_DIR}/../lib)

add_library(${TARGET_NAME} STATIC ${SOURCES} ${LIB_NDI_VAR})
add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${GLES2_INCLUDE_DIR} ${EGL_INCLUDE_DIR} ../)
target_include_directories(native_app_glue PUBLIC
        ${ANDROID_NDK}/sources/android/native_app_glue)

target_link_libraries(${TARGET_NAME} PUBLIC
  ${LIB_NDI_VAR}
  ${GLES2_LIBRARY}
  ${EGL_LIBRARY}
  android
  native_app_glue)
