cmake_minimum_required(VERSION 3.4.1)

include_directories(include)

set(rootPath ${CMAKE_SOURCE_DIR}/../../jniLibs/${CMAKE_ANDROID_ARCH_ABI})
message("rootPath: ${rootPath}" )

add_library(avcodec-57 SHARED IMPORTED)
set_target_properties(avcodec-57 PROPERTIES IMPORTED_LOCATION ${rootPath}/libavcodec-57.so)

add_library(avdevice-57 SHARED IMPORTED)
set_target_properties(avdevice-57 PROPERTIES IMPORTED_LOCATION ${rootPath}/libavdevice-57.so)

add_library(avfilter-6 SHARED IMPORTED)
set_target_properties(avfilter-6 PROPERTIES IMPORTED_LOCATION ${rootPath}/libavfilter-6.so)

add_library(avformat-57 SHARED IMPORTED)
set_target_properties(avformat-57 PROPERTIES IMPORTED_LOCATION ${rootPath}/libavformat-57.so)

add_library(avutil-55 SHARED IMPORTED)
set_target_properties(avutil-55 PROPERTIES IMPORTED_LOCATION ${rootPath}/libavutil-55.so)

add_library(postproc-54 SHARED IMPORTED)
set_target_properties(postproc-54 PROPERTIES IMPORTED_LOCATION ${rootPath}/libpostproc-54.so)

add_library(swresample-2 SHARED IMPORTED)
set_target_properties(swresample-2 PROPERTIES IMPORTED_LOCATION ${rootPath}/libswresample-2.so)

add_library(swscale-4 SHARED IMPORTED)
set_target_properties(swscale-4 PROPERTIES IMPORTED_LOCATION ${rootPath}/libswscale-4.so)

add_library(
        native-lib
        SHARED

        native-lib.cpp
        javaListener.cpp
        thread_test.cpp
        )

target_link_libraries(
        native-lib
        avcodec-57
        avdevice-57
        avfilter-6
        avformat-57
        avutil-55
        postproc-54
        swresample-2
        swscale-4
        log)