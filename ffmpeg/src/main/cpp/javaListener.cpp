//
// Created by yin13 on 2021/3/18.
//

#include "javaListener.h"

JavaListener::JavaListener(JavaVM *javaM, _JNIEnv *env, jobject jo) {
    jvm = javaM;
    jniEnv = env;
    job = jo;

    jclass clz = env->GetObjectClass(jo);
    if (!clz) {
        return;
    }
    jmid = env->GetMethodID(clz, "callError", "(ILjava/lang/String;)V");
}

void JavaListener::onError(int threadType, int code, const char *msg) {
    if (threadType == 1) {
        // 主线程
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(job, jmid, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
    } else if (threadType == 2) {
        // 子线程
        JNIEnv *env;
        jvm->AttachCurrentThread(&env, 0);
        jstring jmsg = env->NewStringUTF(msg);
        env->CallVoidMethod(job, jmid, code, jmsg);
        env->DeleteLocalRef(jmsg);
        jvm->DetachCurrentThread();
    }
}

