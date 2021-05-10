//
// 播放器回调方法
// Created by yin13 on 2021/4/17.
//
#include "TXCallJava.h"

TXCallJava::TXCallJava(JavaVM *vm, JNIEnv *env, jobject *obj) {
    this->javaVm = vm;
    this->jniEnv = env;
    //取指针的值
    this->job = jniEnv->NewGlobalRef(*obj);
    jclass aClass = jniEnv->GetObjectClass(job);
    if (aClass) {
        jmethodId = jniEnv->GetMethodID(aClass, JAVA_METHOD_PARPARED, "()V");
        jmethodIdCallLoad = jniEnv->GetMethodID(aClass, JAVA_METHOD_LOAD, "(Z)V");
        jmethodIdTimeInfo = jniEnv->GetMethodID(aClass, JAVA_METHOD_TIME_INFO, "(II)V");
    }
}

void TXCallJava::onParpared(int type) {
    SDK_LOG_D("type%d: ", type);
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(job, jmethodId);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if (javaVm->AttachCurrentThread(&env, 0) != JNI_OK) {
            return;
        }
        env->CallVoidMethod(job, jmethodId);
        javaVm->DetachCurrentThread();
    }
}

void TXCallJava::onLoad(int type, bool isLoad) {
    SDK_LOG_D("isload%d", isLoad);
    if (type == MAIN_THREAD) {
        jniEnv->CallBooleanMethod(job, jmethodIdCallLoad, isLoad);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if (javaVm->AttachCurrentThread(&env, 0) != JNI_OK) {
            return;
        }
        env->CallBooleanMethod(job, jmethodIdCallLoad, isLoad);
        javaVm->DetachCurrentThread();
    }
}

void TXCallJava::onTimeInfo(int type, int currentTime, int total) {
    SDK_LOG_D("onTimeInfo,currenttime %d ,totaltime %d", currentTime, total);
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(job, jmethodIdTimeInfo, currentTime, total);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if (javaVm->AttachCurrentThread(&env, 0) != JNI_OK) {
            return;
        }
        env->CallVoidMethod(job, jmethodIdTimeInfo, currentTime, total);
        javaVm->DetachCurrentThread();
    }
}
