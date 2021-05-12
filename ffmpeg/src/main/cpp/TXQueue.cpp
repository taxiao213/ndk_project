//
// Created by yin13 on 2021/4/17.
//

#include "TXQueue.h"

TXQueue::TXQueue(TXPlayStatus *status) {
    pthread_cond_init(&pthreadCond, NULL);
    pthread_mutex_init(&pthreadMutex, NULL);
    this->playStatus = status;
}

TXQueue::~TXQueue() {
    clearAvpacket();
    pthread_cond_destroy(&pthreadCond);
    pthread_mutex_destroy(&pthreadMutex);
}

int TXQueue::putAvpacket(AVPacket *avPacket) {
    pthread_mutex_lock(&pthreadMutex);
    queueAvpacket.push(avPacket);
    SDK_LOG_D("放入avpacket, 剩余个数为 %d ", queueAvpacket.size());
    pthread_cond_signal(&pthreadCond);
    pthread_mutex_unlock(&pthreadMutex);
    return 0;
}

int TXQueue::getAvpacket(AVPacket *avPacket) {
    pthread_mutex_lock(&pthreadMutex);
    while (playStatus != NULL && !playStatus->exit) {
        if (queueAvpacket.size() > 0) {
            SDK_LOG_D("getAvpacket 剩余个数为 %d ", queueAvpacket.size());
            AVPacket *&pPacket = queueAvpacket.front();
            // 复制指针
            if (av_packet_ref(avPacket, pPacket) == 0) {
                queueAvpacket.pop();
            }
            av_packet_free(&pPacket);
            av_free(pPacket);
            pPacket = NULL;
            SDK_LOG_D("取出avpacket, 剩余个数为 %d ", queueAvpacket.size());
            break;
        } else {
            pthread_cond_wait(&pthreadCond, &pthreadMutex);
        }
    }
    pthread_mutex_unlock(&pthreadMutex);
    return 0;
}

int TXQueue::getQueueSize() {
    int size = 0;
    pthread_mutex_lock(&pthreadMutex);
    size = queueAvpacket.size();
    pthread_mutex_unlock(&pthreadMutex);
    return size;
}

void TXQueue::clearAvpacket() {
    pthread_cond_signal(&pthreadCond);
    pthread_mutex_lock(&pthreadMutex);
    while (!queueAvpacket.empty()) {
        AVPacket *&pPacket = queueAvpacket.front();
        queueAvpacket.pop();
        av_packet_free(&pPacket);
        av_free(pPacket);
        pPacket = NULL;
    }
    pthread_mutex_unlock(&pthreadMutex);
}
