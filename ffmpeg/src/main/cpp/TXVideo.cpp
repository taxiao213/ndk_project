//
// Created by yin13 on 2021/5/29.
//

#include "TXVideo.h"

TXVideo::TXVideo(TXPlayStatus *playStatus, TXCallJava *callJava) {
    this->txPlayStatus = playStatus;
    this->txCallJava = callJava;
    this->txQueue = new TXQueue(txPlayStatus);
}

TXVideo::~TXVideo() {

}

void *playVideo(void *data) {
    TXVideo *txVideo = (TXVideo *) data;
    if (txVideo != NULL) {
        while (txVideo->txPlayStatus != NULL && !txVideo->txPlayStatus->exit) {
            if (txVideo->txPlayStatus != NULL && txVideo->txPlayStatus->seek) {
                SDK_LOG_D("video 正在seek");
                av_usleep(SLEEP_TIME);
                continue;
            }
            if (txVideo->txQueue != NULL && txVideo->txQueue->getQueueSize() == 0) {
                SDK_LOG_D("video 正在seek");
                if (txVideo->txPlayStatus != NULL && !txVideo->txPlayStatus->load) {
                    txVideo->txPlayStatus->load = true;
                    if (txVideo->txCallJava != NULL) {
                        // 回调
                        SDK_LOG_D("video txCallJava->onLoad 加载");
                        txVideo->txCallJava->onLoad(CHILD_THREAD, true);
                    }
                }
                av_usleep(SLEEP_TIME);
                continue;
            } else {
                if (txVideo->txPlayStatus != NULL && txVideo->txPlayStatus->load) {
                    txVideo->txPlayStatus->load = false;
                    if (txVideo->txCallJava != NULL) {
                        // 回调
                        SDK_LOG_D("video txCallJava->onLoad 取消加载");
                        txVideo->txCallJava->onLoad(CHILD_THREAD, false);
                    }
                }
                AVPacket *pPacket = av_packet_alloc();
                if (txVideo->txQueue->getAvpacket(pPacket) != 0) {
                    SDK_LOG_D("video 获取失败");
                    av_packet_free(&pPacket);
                    av_free(pPacket);
                    pPacket = NULL;
                    continue;
                }

                if (avcodec_send_packet(txVideo->avCodecContext, pPacket) != 0) {
                    SDK_LOG_D("video 0 on success, otherwise negative error code:");
                    av_packet_free(&pPacket);
                    av_free(pPacket);
                    pPacket = NULL;
                    continue;
                }

                AVFrame *pFrame = av_frame_alloc();
                if (avcodec_receive_frame(txVideo->avCodecContext, pFrame) != 0) {
                    SDK_LOG_D("video 0  success, a frame was returned");
                    av_frame_free(&pFrame);
                    av_free(pFrame);
                    pFrame = NULL;

                    av_packet_free(&pPacket);
                    av_free(pPacket);
                    pPacket = NULL;
                    continue;
                }
                SDK_LOG_D("video  解码 avframe 成功");

                av_frame_free(&pFrame);
                av_free(pFrame);
                pFrame = NULL;

                av_packet_free(&pPacket);
                av_free(pPacket);
                pPacket = NULL;
            }
        }
        pthread_exit(&txVideo->pthread);
    }
}

void TXVideo::play() {
    pthread_create(&pthread, NULL, playVideo, this);
}

void TXVideo::release() {
    if (avCodecContext != NULL) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }
    if (avCodecParameters != NULL) {
        avcodec_parameters_free(&avCodecParameters);
        avCodecParameters = NULL;
    }
    if (txCallJava != NULL) {
        txCallJava = NULL;
    }
    if (txPlayStatus != NULL) {
        txPlayStatus = NULL;
    }
    if (txQueue != NULL) {
        delete (txQueue);
        txQueue = NULL;
    }
}
