#include <jni.h>
#include <string>
#include "pthread.h"
#include "android_log.h"
#include "queue"
#include "unistd.h"
#include "javaListener.h"

pthread_t thread;

// 创建线程
void *threadCallBack(void *data) {
    SDK_LOG_D("c++ callback");
    pthread_exit(&thread);
    SDK_LOG_D("c++ callback quit");
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_taxiao_cn_apple_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";

    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_cn_apple_MainActivity_startThread(JNIEnv *env, jobject thiz) {
    pthread_create(&thread, nullptr, threadCallBack, NULL);
}

// 生产者 消费者
pthread_t produce;
pthread_t custom;
pthread_mutex_t mutex;
pthread_cond_t cond;
std::queue<int> queue;

void *produceCallBack(void *data) {
    while (1) {
        SDK_LOG_D("produce: %d " + queue.size());
        pthread_mutex_lock(&mutex);
        queue.push(1);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        sleep(5);
    }

    pthread_exit(&produce);
}

void *customCallBack(void *data) {
    while (1) {
        pthread_mutex_lock(&mutex);
        if (queue.size() > 0) {
            SDK_LOG_D("custom: %d " + queue.size());
            queue.pop();
        } else {
            SDK_LOG_D("custom: wait");
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);
        usleep(1000 * 500);
    }
    pthread_exit(&custom);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_cn_apple_MainActivity_mutexThread(JNIEnv *env, jobject thiz) {

    for (int i = 0; i < 10; ++i) {
        queue.push(i);
    }
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
    pthread_create(&produce, nullptr, produceCallBack, nullptr);
    pthread_create(&custom, nullptr, customCallBack, nullptr);
}


JavaVM *jvm;
JavaListener *javaListener;

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_cn_apple_MainActivity_javaMain2C(JNIEnv *env, jobject jobject1) {
    javaListener = new JavaListener(jvm, env, env->NewGlobalRef(jobject1));
    javaListener->onError(1, 111, "main c++");
}

pthread_t threadCall;

void *threadCallListener(void *data) {
    JavaListener *listener = (JavaListener *) data;
    listener->onError(2, 111, "thread c++");
    pthread_exit(&threadCall);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_cn_apple_MainActivity_javaThread2C(JNIEnv *env, jobject thiz) {
    javaListener = new JavaListener(jvm, env, env->NewGlobalRef(thiz));
    pthread_create(&threadCall, nullptr, threadCallListener, javaListener);
}

// 获取JVM
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jvm = vm;
    JNIEnv *env;
    if (jvm->GetEnv((void **) (&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_6;
}

