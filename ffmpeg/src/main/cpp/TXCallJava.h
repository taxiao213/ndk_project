//
// Created by yin13 on 2021/4/17.
//
#include "jni.h"
#include "android_log.h"

#ifndef APPLE_FFMPEGCALLJAVA_H
#define APPLE_FFMPEGCALLJAVA_H

#define JAVA_METHOD_PARPARED "callParpared"
#define JAVA_METHOD_LOAD "callOnLoad"
#define MAIN_THREAD 1
#define CHILD_THREAD 2

class TXCallJava {
public:
    JavaVM *javaVm;
    JNIEnv *jniEnv;
    jobject job;
    jmethodID jmethodId;
    jmethodID jmethodIdCallLoad;

public:
    TXCallJava(JavaVM *vm, JNIEnv *env, jobject *obj);

    ~TXCallJava();

    void onParpared(int type);

    void onLoad(int type, bool isLoad);
};


#endif //APPLE_FFMPEGCALLJAVA_H
