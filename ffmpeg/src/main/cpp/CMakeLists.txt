cmake_minimum_required(VERSION 3.4.1)

if (FFMPEG_BUILD_VERSION EQUAL 1)
    include_directories(include/low_version)
    set(version low_version)
    set(avcodec libavcodec-57.so)
    set(avdevice libavdevice-57.so)
    set(avfilter libavfilter-6.so)
    set(avformat libavformat-57.so)
    set(avutil libavutil-55.so)
    set(postproc libpostproc-54.so)
    set(swresample libswresample-2.so)
    set(swscale libswscale-4.so)
elseif (FFMPEG_BUILD_VERSION EQUAL 2)
    include_directories(include/high_version)
    set(version high_version)
    set(avcodec libavcodec.so)
    set(avdevice libavdevice.so)
    set(avfilter libavfilter.so)
    set(avformat libavformat.so)
    set(avutil libavutil.so)
    set(postproc libpostproc.so)
    set(swresample libswresample.so)
    set(swscale libswscale.so)
endif ()
set(rootPath ${CMAKE_SOURCE_DIR}/../../jniLibs/${version}/${CMAKE_ANDROID_ARCH_ABI})
message("rootPath: ${rootPath}")

add_library(${avcodec} SHARED IMPORTED)
set_target_properties(${avcodec} PROPERTIES IMPORTED_LOCATION ${rootPath}/${avcodec})

add_library(${avdevice} SHARED IMPORTED)
set_target_properties(${avdevice} PROPERTIES IMPORTED_LOCATION ${rootPath}/${avdevice})

add_library(${avfilter} SHARED IMPORTED)
set_target_properties(${avfilter} PROPERTIES IMPORTED_LOCATION ${rootPath}/${avfilter})

add_library(${avformat} SHARED IMPORTED)
set_target_properties(${avformat} PROPERTIES IMPORTED_LOCATION ${rootPath}/${avformat})

add_library(${avutil} SHARED IMPORTED)
set_target_properties(${avutil} PROPERTIES IMPORTED_LOCATION ${rootPath}/${avutil})

add_library(${postproc} SHARED IMPORTED)
set_target_properties(${postproc} PROPERTIES IMPORTED_LOCATION ${rootPath}/${postproc})

add_library(${swresample} SHARED IMPORTED)
set_target_properties(${swresample} PROPERTIES IMPORTED_LOCATION ${rootPath}/${swresample})

add_library(${swscale} SHARED IMPORTED)
set_target_properties(${swscale} PROPERTIES IMPORTED_LOCATION ${rootPath}/${swscale})

include_directories(soundtouch/source)
aux_source_directory(soundtouch/source DIR_SRCS)

add_library(
        native-lib
        SHARED
        ${DIR_SRCS}
        native-lib.cpp
        javaListener.cpp
        TXCallJava.cpp
        TXFFmpeg.cpp
        TXAudio.cpp
        TXQueue.cpp
        TXPlayStatus.cpp
        TXCallBack.cpp
        TXBufferQueue.cpp
        TXPcmBean.cpp
        test/thread_test.cpp
        test/Opensles_test.cpp
        TXVideo.cpp
)

target_link_libraries(
        native-lib
        ${avcodec}
        ${avdevice}
        ${avfilter}
        ${avformat}
        ${avutil}
        ${postproc}
        ${swresample}
        ${swscale}
        OpenSLES
        log)