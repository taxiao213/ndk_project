//
// Created by yin13 on 2021/3/18.
//

#include "jni.h"

#ifndef APPLE_JAVALISTENER_H
#define APPLE_JAVALISTENER_H


class JavaListener {
public :
    JavaVM *jvm;
    _JNIEnv *jniEnv;
    jobject job;
    jmethodID jmid;

public:
    JavaListener(JavaVM *javaM, _JNIEnv *jniEnv, jobject job);

    ~JavaListener();

    /**
     *
     * @param threadType  1 主线程 2 子线程
     * @param code
     * @param msg
     */
    void onError(int threadType, int code, const char *msg);
};


#endif //APPLE_JAVALISTENER_H
