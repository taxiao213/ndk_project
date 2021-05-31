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
                if (pFrame->format == AV_PIX_FMT_YUV420P) {
                    // 直接渲染
                    SDK_LOG_D("video 格式 YUV420P");
                    if (txVideo->txCallJava != NULL) {
                        // 回调
                        SDK_LOG_D("video txCallJava->onCallOnRenderYUV");
                        txVideo->txCallJava->onCallOnRenderYUV(CHILD_THREAD,
                                                               txVideo->avCodecContext->width,
                                                               txVideo->avCodecContext->height,
                                                               pFrame->data[0], pFrame->data[1],
                                                               pFrame->data[2]);
                    }

                } else {
                    // 转成格式为 YUV420P 格式
                    SDK_LOG_D("video 转成格式为 YUV420P 格式");
                    // 申请空间
                    AVFrame *pFrame420P = av_frame_alloc();
                    // 获取 buffer
                    int num = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                                       txVideo->avCodecContext->width,
                                                       txVideo->avCodecContext->height, 1);
                    uint8_t *buffer = reinterpret_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));
                    // 数据填充
                    av_image_fill_arrays(pFrame420P->data, pFrame420P->linesize, buffer,
                                         AV_PIX_FMT_YUV420P, txVideo->avCodecContext->width,
                                         txVideo->avCodecContext->height, 1);
                    // 转换格式为 YUV420P
                    SwsContext *swsContext = sws_getContext(txVideo->avCodecContext->width,
                                                            txVideo->avCodecContext->height,
                                                            txVideo->avCodecContext->pix_fmt,
                                                            txVideo->avCodecContext->width,
                                                            txVideo->avCodecContext->height,
                                                            AV_PIX_FMT_YUV420P, SWS_BICUBIC,
                                                            NULL, NULL, NULL);
                    if (!swsContext) {
                        av_frame_free(&pFrame420P);
                        av_free(pFrame420P);
                        pFrame420P = NULL;
                        av_free(buffer);
                        buffer = NULL;
                        continue;
                    }
                    sws_scale(swsContext, pFrame->data,
                              pFrame->linesize, 0, pFrame->height,
                              pFrame420P->data, pFrame420P->linesize);
                    // 已经转换成YUV420P,直接渲染
                    if (txVideo->txCallJava != NULL) {
                        SDK_LOG_D("video txCallJava->onCallOnRenderYUV");
                        txVideo->txCallJava->onCallOnRenderYUV(CHILD_THREAD,
                                                               txVideo->avCodecContext->width,
                                                               txVideo->avCodecContext->height,
                                                               pFrame420P->data[0],
                                                               pFrame420P->data[1],
                                                               pFrame420P->data[2]);
                    }
                    av_frame_free(&pFrame420P);
                    av_free(pFrame420P);
                    pFrame420P = NULL;
                    av_free(buffer);
                    buffer = NULL;
                    sws_freeContext(swsContext);
                    swsContext = NULL;
                }
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
