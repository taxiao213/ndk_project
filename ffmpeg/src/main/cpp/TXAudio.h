//
// Created by yin13 on 2021/4/17.
//
#include "TXQueue.h"
#include "TXPlayStatus.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}
#ifndef APPLE_TXAUDIO_H
#define APPLE_TXAUDIO_H

class TXAudio {
public:
    int streamIndex = -1;
    AVCodecParameters *codecpar = NULL;
    AVCodecContext *pCodecContext = NULL;
    TXQueue *queue = NULL;
    TXPlayStatus *playStatus = NULL;
    pthread_t  p_thread = NULL;
    AVPacket *avPacket = NULL;
    int ret = -1;
    AVFrame *avFrame = NULL;
    uint8_t *buffer = NULL;
    int data_size = -1;
public:
    TXAudio(TXPlayStatus *txPlayStatus);

    ~TXAudio();

    void play();

    int resampleAudio();
};


#endif //APPLE_TXAUDIO_H
