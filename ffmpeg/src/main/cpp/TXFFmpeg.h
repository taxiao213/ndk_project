//
// Created by yin13 on 2021/4/17.
//
#include "TXCallJava.h"
#include "pthread.h"
#include "string.h"
#include "TXAudio.h"

#ifndef APPLE_TXFFMPEG_H
#define APPLE_TXFFMPEG_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
}

class TXFFmpeg {
public:
    TXCallJava *callJava = NULL;
    pthread_t decodeThread = NULL;
    const char *url = NULL;
    AVFormatContext *pContext = NULL;
    TXAudio *pAudio = NULL;
    TXPlayStatus *playStatus = NULL;
    pthread_mutex_t initMutex;
    bool exit = false;
public:
    TXFFmpeg(TXCallJava *txCallJava, TXPlayStatus *txPlayStatus, const char *url);

    ~TXFFmpeg();

    void parpared();

    // 解码器
    void decodedFFmpegThread();

    void start();

    void resume();

    void pause();

    void release();
};


#endif //APPLE_TXFFMPEG_H
