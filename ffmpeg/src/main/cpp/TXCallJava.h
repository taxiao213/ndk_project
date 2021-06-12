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
#define JAVA_METHOD_RENDER_YUV "callOnRenderYUV"
#define JAVA_METHOD_IS_SUPPORT_MEDIA_CODEC "callOnIsSupportMediaCodec"
#define JAVA_METHOD_INIT_MEDIA_CODEC_VIDEO "callOnInitMediaCodecVideo"
#define JAVA_METHOD_DECODE_AV_PACKET "callOnDecodeAvPacket"

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
    jmethodID jmethodIdRenderYUV;
    jmethodID jmethodIdIsSupportMediaCodec;
    jmethodID jmethodIdInitMediaCodecVideo;
    jmethodID jmethodIdDecodeAvPacket;

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

    void onCallOnRenderYUV(int type, int width, int height, uint8_t *y, uint8_t *u, uint8_t *v);

    bool onCallIsSupportMediaCodec(int type, const char *s);

    void
    onCallInitMediaCodecVideo(int type, const char *mime, int width, int height, int size,
                                uint8_t *csd_01,
                              int size2,
                                uint8_t *csd_02);

    void onCallDecodeAvPacket(int type, int size, uint8_t *buffer);
};


#endif //APPLE_FFMPEGCALLJAVA_H
