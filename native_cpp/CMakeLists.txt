cmake_minimum_required(VERSION 3.18.1)
project(video-app C CXX)
set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME ndi-monitor)

include_directories(${GLIB_INCLUDE_DIRS})

if (ANDROID)
    set(LIB_NDI_VAR "/home/iaroslav/work/streaming/NDI/Android/NDI\ Advanced\ SDK\ for\ Android/lib/arm64-v8a/libndi.so")
elseif (LINUX)
    set(LIB_NDI_VAR "/home/iaroslav/work/streaming/NDI/5.5.3/NDI-Advanced-SDK-for-Linux/lib/x86_64-linux-gnu/libndi.so.5.5.3")
else ()
    set(LIB_NDI_VAR "")
endif ()

add_subdirectory(ndi-rx-scan)
add_subdirectory(ndi-rx)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.[h,c]pp")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_library(${TARGET_NAME} SHARED ${SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ndi-rx-scan ndi-app)
target_link_libraries(${TARGET_NAME} PUBLIC ndi_rx_scan ndi_app_rx)
