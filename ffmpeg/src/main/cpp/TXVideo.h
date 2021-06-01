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
    double clock = 0;
    double delayTime = 0;
    double defaultDelayTime = 0;
public:
    TXVideo(TXPlayStatus *playStatus, TXCallJava *callJava);

    ~TXVideo();

    void play();

    void release();

    double getFrameDiffTime(AVFrame *avFrame);

    double getDelayTime(double diff);
};


#endif //APPLE_TXVIDEO_H
