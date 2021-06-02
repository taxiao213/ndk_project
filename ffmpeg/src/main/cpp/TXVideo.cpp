//
// Created by yin13 on 2021/5/29.
//

#include "TXVideo.h"

TXVideo::TXVideo(TXPlayStatus *playStatus, TXCallJava *callJava) {
    this->txPlayStatus = playStatus;
    this->txCallJava = callJava;
    this->txQueue = new TXQueue(txPlayStatus);
    pthread_mutex_init(&pthreadMutex, NULL);
}

TXVideo::~TXVideo() {
    pthread_mutex_destroy(&pthreadMutex);
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

            if (txVideo->txPlayStatus != NULL && txVideo->txPlayStatus->pause) {
                SDK_LOG_D("video 正在pause");
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
                pthread_mutex_lock(&txVideo->pthreadMutex);
                if (avcodec_send_packet(txVideo->avCodecContext, pPacket) != 0) {
                    SDK_LOG_D("video 0 on success, otherwise negative error code:");
                    av_packet_free(&pPacket);
                    av_free(pPacket);
                    pPacket = NULL;
                    pthread_mutex_unlock(&txVideo->pthreadMutex);
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
                    pthread_mutex_unlock(&txVideo->pthreadMutex);
                    continue;
                }
                SDK_LOG_D("video  解码 avframe 成功");
                double diffTime = txVideo->getFrameDiffTime(pFrame);
                SDK_LOG_D("video  diffTime1 %f ", diffTime);
                double delayTime = txVideo->getDelayTime(diffTime);
                av_usleep(delayTime);
                SDK_LOG_D("video  diffTime2 %f ", delayTime);
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
                        pthread_mutex_unlock(&txVideo->pthreadMutex);
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
                pthread_mutex_unlock(&txVideo->pthreadMutex);
            }
        }
        pthread_exit(&txVideo->pthread);
    }
}

void TXVideo::play() {
    if (txPlayStatus != NULL) {
        txPlayStatus->pause = false;
    }
    pthread_create(&pthread, NULL, playVideo, this);
}

void TXVideo::release() {
    if (txQueue != NULL) {
        delete (txQueue);
        txQueue = NULL;
    }
    if (avCodecContext != NULL) {
        pthread_mutex_lock(&pthreadMutex);
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
        pthread_mutex_unlock(&pthreadMutex);
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
}

/**
 * 音视频同步  以音频为主
 * @param avFrame
 * @return
 */
double TXVideo::getFrameDiffTime(AVFrame *avFrame) {
    double pts = av_frame_get_best_effort_timestamp(avFrame);
    if (pts == AV_NOPTS_VALUE) {
        pts = 0;
    }
    //分别获取音频和视频的PTS（播放时间戳）
    pts *= av_q2d(time_base);
    if (pts > 0) {
        clock = pts;
    }
    double diff = txAudio->clock - clock;
    return diff;
}

// 计算延迟时间
double TXVideo::getDelayTime(double diff) {
    // 音频快
    if (diff > 0.003) {
        delayTime = delayTime * 2 / 3;

    } else if (diff < -0.003) {
        // 视频快
        delayTime = delayTime * 3 / 2;
    } else if (diff == 0.003) {

    }
    if (diff >= 0.5) {
        delayTime = 0;
    } else if (diff <= -0.5) {
        delayTime = defaultDelayTime * 2;
    }
    if (fabs(diff) >= 10) {
        delayTime = defaultDelayTime;
    }
    return delayTime;
}

void TXVideo::resume() {
    if (txPlayStatus != NULL) {
        txPlayStatus->pause = false;
    }
}

void TXVideo::pause() {
    if (txPlayStatus != NULL) {
        txPlayStatus->pause = true;
    }
}
