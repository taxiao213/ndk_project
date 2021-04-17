//
// Created by yin13 on 2021/4/17.
//
#include "TXQueue.h"
#include "TXPlayStatus.h"

extern "C" {
#include <libavformat/avformat.h>
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
public:
    TXAudio(TXPlayStatus *txPlayStatus);

    ~TXAudio();
};


#endif //APPLE_TXAUDIO_H
