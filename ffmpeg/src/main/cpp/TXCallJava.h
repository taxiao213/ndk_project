//
// Created by yin13 on 2021/4/17.
//
#include "jni.h"
#include "android_log.h"

#ifndef APPLE_FFMPEGCALLJAVA_H
#define APPLE_FFMPEGCALLJAVA_H

#define JAVA_METHOD_PARPARED "callParpared"
#define JAVA_METHOD_LOAD "callOnLoad"
#define JAVA_METHOD_TIME_INFO "callTimeInfo"
#define JAVA_METHOD_ERROR "callOnError"
#define JAVA_METHOD_COMPLETE "callOnComplete"
#define JAVA_METHOD_VALUME_DB "callOnValumeDB"
#define JAVA_METHOD_PCM_AAC "callOnPcmTAAc"
#define JAVA_METHOD_RECORD_TIME "callOnRecordTime"
#define JAVA_METHOD_CUT_AUDIO "callOnCutAudio"
#define MAIN_THREAD 1
#define CHILD_THREAD 2

class TXCallJava {
public:
    JavaVM *javaVm;
    JNIEnv *jniEnv;
    jobject job;
    jmethodID jmethodId;
    jmethodID jmethodIdCallLoad;
    jmethodID jmethodIdTimeInfo;
    jmethodID jmethodIdError;
    jmethodID jmethodIdComplete;
    jmethodID jmethodIdValumeDB;
    jmethodID jmethodIdPcmAAc;
    jmethodID jmethodIdRecordTime;
    jmethodID jmethodIdCutAudio;

public:
    TXCallJava(JavaVM *vm, JNIEnv *env, jobject *obj);

    ~TXCallJava();

    void onParpared(int type);

    void onLoad(int type, bool isLoad);

    void onTimeInfo(int type, int currentTime, int total);

    void onError(int type, int code, char *errorMsg);

    void onCallComplete(int type);

    void onCallValumeDB(int type, int db);

    void onCallOnPcmTAAc(int type, int size, void *pcmBuffer);

    void onCallOnRecordTime(int type, float time);

    void onCallOnCutAudio(int type, int sampleRate, int size, void *pcmBuffer);
};


#endif //APPLE_FFMPEGCALLJAVA_H
