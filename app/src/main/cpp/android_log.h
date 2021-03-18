//
// Created by yin13 on 2021/3/17.
//

#ifndef APPLE_ANDROID_LOG_H
#define APPLE_ANDROID_LOG_H

#endif //APPLE_ANDROID_LOG_H

#include "android/log.h"

const char *TAG = "TA_XIAO";

#define SDK_LOG_V(FORMAT, ...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, "[%s] line: %d info: " FORMAT, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)

#define SDK_LOG_D(FORMAT, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, "[%s] line: %d info: " FORMAT, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)

#define SDK_LOG_W(FORMAT, ...) __android_log_print(ANDROID_LOG_WARN, TAG, "[%s] line: %d info: " FORMAT, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)

#define SDK_LOG_E(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, "[%s] line: %d info: " FORMAT, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)

