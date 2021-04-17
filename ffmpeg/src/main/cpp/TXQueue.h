//
// Created by yin13 on 2021/4/17.
//
#include "pthread.h"
#include "queue"
#include "android_log.h"
#include "TXPlayStatus.h"
#include <stdlib.h>

extern "C" {
#include <libavcodec/avcodec.h>
}
#ifndef APPLE_TXQUEUE_H
#define APPLE_TXQUEUE_H


class TXQueue {
public:
    std::queue<AVPacket *> queueAvpacket;
    pthread_cond_t pthreadCond;
    pthread_mutex_t pthreadMutex;
    TXPlayStatus *playStatus;
public:
    TXQueue(TXPlayStatus* status);

    ~TXQueue();

    int putAvpacket(AVPacket *avPacket);

    int getAvpacket(AVPacket *avPacket);

    int getQueueSize();
};


#endif //APPLE_TXQUEUE_H
