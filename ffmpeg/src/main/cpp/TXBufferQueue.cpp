//
// Created by yin13 on 2021/5/26.
//

#include "TXBufferQueue.h"

TXBufferQueue::TXBufferQueue(TXPlayStatus *playStatus) {
    this->txPlayStatus = playStatus;
    pthread_mutex_init(&pthreadMutex, NULL);
    pthread_cond_init(&pthreadCond, NULL);
}

TXBufferQueue::~TXBufferQueue() {
    this->txPlayStatus = NULL;
    pthread_mutex_destroy(&pthreadMutex);
    pthread_cond_destroy(&pthreadCond);
}

void TXBufferQueue::release() {
    noticeThread();
    clearBuffer();
}

void TXBufferQueue::putBuffer(SAMPLETYPE *sampleBuffer, int size) {
    SDK_LOG_D("putBuffer size %d", size);
    pthread_mutex_lock(&pthreadMutex);
    TXPcmBean *txPcmBean = new TXPcmBean(size, sampleBuffer);
    queueBuffer.push_back(txPcmBean);
    pthread_cond_signal(&pthreadCond);
    pthread_mutex_unlock(&pthreadMutex);
}

void TXBufferQueue::getBuffer(TXPcmBean **pTxPcmBean) {
    pthread_mutex_lock(&pthreadMutex);
    while (txPlayStatus != NULL && !txPlayStatus->exit) {
        if (queueBuffer.size() > 0) {
            SDK_LOG_D("getBuffer ");
            *pTxPcmBean = queueBuffer.front();
            queueBuffer.pop_front();
            break;
        } else {
            if (txPlayStatus != NULL && !txPlayStatus->exit) {
                SDK_LOG_D("getBuffer wait");
                pthread_cond_wait(&pthreadCond, &pthreadMutex);
            }
        }
    }
    pthread_mutex_unlock(&pthreadMutex);
}

void TXBufferQueue::clearBuffer() {
    pthread_cond_signal(&pthreadCond);
    pthread_mutex_lock(&pthreadMutex);
    while (queueBuffer.size() > 0) {
        TXPcmBean *pBean = queueBuffer.front();
        delete (pBean);
        pBean = NULL;
        queueBuffer.pop_front();
    }
    pthread_mutex_unlock(&pthreadMutex);
}

void TXBufferQueue::noticeThread() {
    pthread_cond_signal(&pthreadCond);
}

int TXBufferQueue::getBufferSize() {
    int size = 0;
    pthread_mutex_lock(&pthreadMutex);
    size = queueBuffer.size();
    pthread_mutex_unlock(&pthreadMutex);
    return size;
}
