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
bool isExit = false;
pthread_t thread_start;

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
    ThreadTest *threadTest = new ThreadTest(&thread, "测试");
}

// 生产者 消费者
pthread_t produce;
pthread_t custom;
pthread_mutex_t mutex;
pthread_cond_t cond;
std::queue<int> queue;
JavaVM *jvm;
JavaListener *javaListener;

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

void *ffmpegStart(void *data) {
    SDK_LOG_D("ffmpegStart");
    TXFFmpeg *txfFmpeg = (TXFFmpeg *) data;
    if (txfFmpeg != NULL) {
        txfFmpeg->start();
    }
    pthread_exit(&thread_start);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1start(JNIEnv *env, jobject thiz) {
    if (isExit)return;
    if (ffmpeg != NULL) {
        pthread_create(&thread_start, NULL, ffmpegStart, ffmpeg);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1resume(JNIEnv *env, jobject thiz) {
    if (ffmpeg != NULL) {
        ffmpeg->resume();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1pause(JNIEnv *env, jobject thiz) {
    if (ffmpeg != NULL) {
        ffmpeg->pause();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1stop(JNIEnv *env, jobject thiz) {
    // 开子线程去释放
    if (isExit) return;
    isExit = true;
    if (ffmpeg != NULL) {
        SDK_LOG_D("release start00");
        ffmpeg->release();
        SDK_LOG_D("release start11");
        delete (ffmpeg);
        SDK_LOG_D("release start22");
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
    isExit = false;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1seek(JNIEnv *env, jobject thiz, jint seconds) {
    if (ffmpeg != NULL) {
        ffmpeg->setSeek(seconds);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1volume(JNIEnv *env, jobject thiz, jint percent) {
    if (ffmpeg != NULL) {
        ffmpeg->setVolume(percent);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1mute(JNIEnv *env, jobject thiz, jint channel) {
    // 设置声道
    if (ffmpeg != NULL) {
        ffmpeg->setMute(channel);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1pitch(JNIEnv *env, jobject thiz, jfloat pitch) {
    if (ffmpeg != NULL) {
        ffmpeg->setPitch(pitch);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1speed(JNIEnv *env, jobject thiz, jfloat speed) {
    if (ffmpeg != NULL) {
        ffmpeg->setSpeed(speed);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1getSampleRate(JNIEnv *env, jobject thiz) {
    if (ffmpeg != NULL) {
        return ffmpeg->getSampleRate();
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1startRecord(JNIEnv *env, jobject thiz) {
    if (ffmpeg != NULL) {
        ffmpeg->startRecord();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1resumeRecord(JNIEnv *env, jobject thiz,
                                                  jboolean resumeRecord) {
    if (ffmpeg != NULL) {
        ffmpeg->resumeRecord(resumeRecord);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1stopRecord(JNIEnv *env, jobject thiz) {
    if (ffmpeg != NULL) {
        ffmpeg->stopRecord();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_n_1cutAudio(JNIEnv *env, jobject thiz, jint start_time,
                                              jint end_time, jboolean is_show_pcm,
                                              jstring filePath) {
    if (ffmpeg != NULL) {
        const char *url = env->GetStringUTFChars(filePath, 0);
        ffmpeg->cutAudio(start_time, end_time, is_show_pcm, url);
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

#include "TXCallBack.h";

extern "C" {
#include <libavutil/time.h>
};

void setCallBack(TXCallBack *txCallBack) {
    if (txCallBack != NULL) {
        SDK_LOG_D("size:%d", txCallBack->buffer_size);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_taxiao_ffmpeg_JniSdkImpl_test(JNIEnv *env, jobject thiz) {
    TXCallBack *txCallBack = new TXCallBack(1, NULL);

    void *cv = NULL;
    int size = 0;
    while (true) {
        av_usleep(1000 * 1000);
        txCallBack->buffer_size = size;
        txCallBack->buffer = cv;
        setCallBack(txCallBack);
        size++;
    }

}
