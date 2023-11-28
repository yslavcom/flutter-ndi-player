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

if (ANDROID)
    set(ffmpeg_dir /home/iaroslav/Downloads/personal/Flutter/prj/TestDrive/ffmpeg-android-maker/output)
    set(ffmpeg_libs ${ffmpeg_dir}/lib/${ANDROID_ABI})
    set(ffmpeg_include_dir ${ffmpeg_dir}/include/${ANDROID_ABI})
    include_directories(${ffmpeg_include_dir})

    set(
        # List variable name
        ffmpeg_libs_names
        # Values in the list
        avutil avformat avcodec swscale swresample)

    foreach (ffmpeg_lib_name ${ffmpeg_libs_names})
        add_library(
            ${ffmpeg_lib_name}
            SHARED
            IMPORTED)
        set_target_properties(
            ${ffmpeg_lib_name}
            PROPERTIES
            IMPORTED_LOCATION
            ${ffmpeg_libs}/lib${ffmpeg_lib_name}.so)
    endforeach ()
endif ()

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
    target_link_libraries(${TARGET_NAME} PUBLIC  ${log-lib}  ${ffmpeg_libs_names})
endif ()

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${ffmpeg_include_dir} ../)
