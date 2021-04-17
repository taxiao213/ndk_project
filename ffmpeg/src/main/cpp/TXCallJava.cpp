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
