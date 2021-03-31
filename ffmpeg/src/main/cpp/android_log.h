//
// Created by yin13 on 2021/3/17.
//
#include "android/log.h"

#ifndef APPLE_ANDROID_LOG_H
#define APPLE_ANDROID_LOG_H
#endif //APPLE_ANDROID_LOG_H

#define TAG "TA_XIAO"
#define DEBUG 1
#if DEBUG
    #define SDK_LOG_V(FORMAT, ...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, "[%s] line: %d info: " FORMAT, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define SDK_LOG_D(FORMAT, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, "[%s] line: %d info: " FORMAT, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define SDK_LOG_W(FORMAT, ...) __android_log_print(ANDROID_LOG_WARN, TAG, "[%s] line: %d info: " FORMAT, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define SDK_LOG_E(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, "[%s] line: %d info: " FORMAT, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
    #define SDK_LOG_V(FORMAT, ...)
    #define SDK_LOG_D(FORMAT, ...)
    #define SDK_LOG_W(FORMAT, ...)
    #define SDK_LOG_E(FORMAT, ...)
#endif
