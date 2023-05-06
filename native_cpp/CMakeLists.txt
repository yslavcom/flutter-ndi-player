cmake_minimum_required(VERSION 3.18.1)
project(video-app C CXX)
set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME ndi-monitor)

include_directories(${GLIB_INCLUDE_DIRS})

set(CMAKE_BUILD_TYPE Debug)

if (ANDROID)
    set(LIB_NDI_VAR "/home/iaroslav/work/streaming/NDI/Android/NDI\ Advanced\ SDK\ for\ Android/lib/${ANDROID_ABI}/libndi.so")
elseif (LINUX)
    set(LIB_NDI_VAR "")
else ()
    set(LIB_NDI_VAR "")
endif ()

add_subdirectory(common)
add_subdirectory(ndi-rx-scan)
add_subdirectory(ndi-rx)
add_subdirectory(player)
add_subdirectory(rx-frames-controller)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.[h,c]pp")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_library(${TARGET_NAME} SHARED ${SOURCES})

if (ANDROID)
    target_compile_definitions(${TARGET_NAME} PRIVATE ANDROID_OUT=1)
elseif (LINUX)
    target_compile_definitions(${TARGET_NAME} PRIVATE LINUX_OUT=1)
else ()
    target_compile_definitions(${TARGET_NAME} PRIVATE OTHER_OUT=1)
endif ()

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    common
    ndi-rx-scan
    ndi-app
    player
    rx-frames-controller
    )
target_link_libraries(${TARGET_NAME} PUBLIC
    ndi_rx_scan
    ndi_app_rx
    common
    player
    rx_frames_controller
    )
