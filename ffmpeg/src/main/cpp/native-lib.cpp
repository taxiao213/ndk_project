#include <jni.h>
#include <string>

#include "pthread.h"
#include "android_log.h"
#include "queue"
#include "unistd.h"
#include "javaListener.h"
#include "test/thread_test.h"
#include "TXFFmpeg.h"
#include "TXCallJava.h"

extern "C" {
#include <libavformat/avformat.h>
}

//#include <SLES/OpenSLES.h>
//#include <SLES/OpenSLES_Android.h>
//#include "assert.h"

#include "test/Opensles_test.h"


pthread_t thread = NULL;
TXFFmpeg *ffmpeg = NULL;
TXCallJava *callJava = NULL;
TXPlayStatus *txPlayStatus = NULL;

// 创建线程
void *threadCallBack(void *data) {
    SDK_LOG_D("c++ callback");
    pthread_exit(&thread);
    SDK_LOG_D("c++ callback quit");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_startThread(JNIEnv *env, jobject thiz) {
//    pthread_create(&thread, nullptr, threadCallBack, NULL);
    ThreadTest *threadTest = new ThreadTest(&thread, "的多大");
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
Java_com_taxiao_ffmpeg_JniSdkImpl_mutexThread(JNIEnv *env, jobject thiz) {

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
Java_com_taxiao_ffmpeg_JniSdkImpl_javaMain2C(JNIEnv *env, jobject jobject1) {
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
Java_com_taxiao_ffmpeg_JniSdkImpl_javaThread2C(JNIEnv *env, jobject thiz) {
    javaListener = new JavaListener(jvm, env, env->NewGlobalRef(thiz));
    pthread_create(&threadCall, nullptr, threadCallListener, javaListener);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_testFFmpeg(JNIEnv *env, jobject thiz) {
//    int init = avformat_network_init();
//    int deinit = avformat_network_deinit();
//
//    SDK_LOG_D("init: %d , deinit: %d", init, deinit);

    AVCodec *pt_avCodec = av_codec_next(NULL);
    while (pt_avCodec != NULL) {
        switch (pt_avCodec->type) {
            case AVMEDIA_TYPE_VIDEO:
                SDK_LOG_D("codec:%s", (pt_avCodec->name));
                break;
            case AVMEDIA_TYPE_AUDIO:
                SDK_LOG_D("codec:%s", (pt_avCodec->name));
                break;
        }
        pt_avCodec = pt_avCodec->next;
    }
}

// --------------------------------- ffmpeg ---------------------------- //

// 获取JVM
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jvm = vm;
    JNIEnv *env;
    if (jvm->GetEnv((void **) (&env), JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_4;
}

/**
 * ffmpeg 初始化
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1parpared(JNIEnv *env, jobject thiz, jstring path) {
    const char *url = env->GetStringUTFChars(path, 0);
    if (ffmpeg == NULL) {
        if (callJava == NULL) {
            callJava = new TXCallJava(jvm, env, &thiz);
        }
        if (txPlayStatus == NULL) {
            txPlayStatus = new TXPlayStatus();
        }
        ffmpeg = new TXFFmpeg(callJava, txPlayStatus, url);
    }
    ffmpeg->parpared();
    env->ReleaseStringUTFChars(path, url);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_start(JNIEnv *env, jobject thiz) {
    if (ffmpeg != NULL) {
        SDK_LOG_D("start");
        ffmpeg->start();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_resume(JNIEnv *env, jobject thiz) {
    if (ffmpeg != NULL) {
        ffmpeg->resume();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_pause(JNIEnv *env, jobject thiz) {
    if (ffmpeg != NULL) {
        ffmpeg->pause();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_stop(JNIEnv *env, jobject thiz) {
    if (ffmpeg != NULL) {
        ffmpeg->release();
        delete (ffmpeg);
        ffmpeg = NULL;
    }
    if (callJava != NULL) {
        delete (callJava);
        callJava = NULL;
    }
    if (txPlayStatus != NULL) {
        delete (txPlayStatus);
        txPlayStatus = NULL;
    }
}

//------------------------------- OpenSLES pcm -------------------------//

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_testPlay(JNIEnv *env, jobject thiz, jstring path) {
    const char *url = env->GetStringUTFChars(path, 0);
    new Opensles_test{url};
    env->ReleaseStringUTFChars(path, url);
}
