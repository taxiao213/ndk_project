//
// Created by yin13 on 2021/5/29.
//

#ifndef APPLE_TXVIDEO_H
#define APPLE_TXVIDEO_H

#include "TXCallJava.h"
#include "TXQueue.h"
#include "TXAudio.h"

extern "C" {
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};

#define CODEC_YUV 0
#define CODEC_MEDIACODEC 1

class TXVideo {
public:
    int streamVideoIndex = -1;
    AVCodecContext *avCodecContext = NULL;
    AVCodecParameters *avCodecParameters = NULL;
    TXCallJava *txCallJava = NULL;
    TXPlayStatus *txPlayStatus = NULL;
    TXQueue *txQueue = NULL;
    AVRational time_base;
    pthread_t pthread;
    TXAudio *txAudio;
    pthread_mutex_t pthreadMutex;
    double clock = 0;
    double delayTime = 0;
    double defaultDelayTime = 0;
    int codecType = 0;
    AVBSFContext *abs_ctx = NULL;
public:
    TXVideo(TXPlayStatus *playStatus, TXCallJava *callJava);

    ~TXVideo();

    void play();

    void release();

    double getFrameDiffTime(AVFrame *avFrame, AVPacket *avPacket);

    double getDelayTime(double diff);

    void resume();

    void pause();
};


#endif //APPLE_TXVIDEO_H
