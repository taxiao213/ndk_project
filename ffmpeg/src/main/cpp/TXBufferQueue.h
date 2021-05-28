//
// Created by yin13 on 2021/5/26.
//

#ifndef APPLE_TXBUFFERQUEUE_H
#define APPLE_TXBUFFERQUEUE_H

#include "deque"
#include "TXPlayStatus.h"
#include "TXPcmBean.h"
#include "pthread.h"
#include "soundtouch/source/SoundTouch.h"
#include "android_log.h"

extern "C" {
#include <libavcodec/avcodec.h>
};
using namespace soundtouch;

class TXBufferQueue {
public:
    std::deque<TXPcmBean *> queueBuffer;
    pthread_mutex_t pthreadMutex;
    pthread_cond_t pthreadCond;
    TXPlayStatus *txPlayStatus = NULL;
public:
    TXBufferQueue(TXPlayStatus *playStatus);

    ~TXBufferQueue();

    void release();

    void putBuffer(SAMPLETYPE *sampleBuffer, int size);

    void getBuffer(TXPcmBean** pTxPcmBean);

    void clearBuffer();

    int getBufferSize();

    void noticeThread();
};


#endif //APPLE_TXBUFFERQUEUE_H
