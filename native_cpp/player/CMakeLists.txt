# cmake_minimum_required(VERSION 3.18.1)
cmake_minimum_required(VERSION 3.16.3)

project(
  ndi-monitor
  VERSION 1.0
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME player)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g0")

# Check if the platform is Android
if(ANDROID)
    add_definitions(-DANDROID_PLATFORM)  # Define a preprocessor macro
elseif(UNIX AND NOT APPLE)
    add_definitions(-DLINUX_PLATFORM)
endif()

add_compile_options(-fPIC)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.[h,c]pp")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# GLESv2
find_path(GLES2_INCLUDE_DIR GLES2/gl2.h HINTS ${ANDROID_NDK})
find_library(GLES2_LIBRARY libGLESv2.so HINTS ${GLES2_INCLUDE_DIR}/../lib)

if(ANDROID)
  find_library(MEDIA_NDK_LIB mediandk)
endif ()

# EGL
find_path(EGL_INCLUDE_DIR EGL/egl.h HINTS ${ANDROID_NDK})
find_library(EGL_LIBRARY libEGL.so HINTS ${EGL_INCLUDE_DIR}/../lib)

add_library(${TARGET_NAME} STATIC ${SOURCES} ${LIB_NDI_VAR})
if (ANDROID)
  add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
endif()

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${GLES2_INCLUDE_DIR} ${EGL_INCLUDE_DIR} ../)
if (ANDROID)
  target_include_directories(native_app_glue PUBLIC
        ${ANDROID_NDK}/sources/android/native_app_glue)
endif()

if(ANDROID)
  target_link_libraries(${TARGET_NAME} PUBLIC
    ${LIB_NDI_VAR}
    ${GLES2_LIBRARY}
    ${EGL_LIBRARY}
    android
    native_app_glue
    ${MEDIA_NDK_LIB})
else()
  target_link_libraries(${TARGET_NAME} PUBLIC
    ${LIB_NDI_VAR}
    ${GLES2_LIBRARY}
    ${EGL_LIBRARY})
endif()
