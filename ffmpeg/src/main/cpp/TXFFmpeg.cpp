//
// Created by yin13 on 2021/4/17.
//

#include "TXFFmpeg.h"


TXFFmpeg::TXFFmpeg(TXCallJava *txCallJava, TXPlayStatus *txPlayStatus, const char *url) {
    this->callJava = txCallJava;
    this->url = url;
    this->playStatus = txPlayStatus;
    pthread_mutex_init(&initMutex, NULL);
}

void *decodeFFmpeg(void *data) {
    TXFFmpeg *txfFmpeg = (TXFFmpeg *) (data);
    txfFmpeg->decodedFFmpegThread();
    pthread_exit(&txfFmpeg->decodeThread);
}

void TXFFmpeg::parpared() {
    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);
}

// 失败的返回值
int ffmpegCallBack(void *pVoid) {
    TXFFmpeg *txfFmpeg = (TXFFmpeg *) pVoid;
    if (txfFmpeg->playStatus->exit) {
        return AVERROR_EOF;
    }
    return 0;
}

void TXFFmpeg::decodedFFmpegThread() {
    pthread_mutex_lock(&initMutex);
    // 注册解码器并初始化网络
    av_register_all();
    avformat_network_init();
    // 打开文件或者网络流
    pContext = avformat_alloc_context();

    pContext->interrupt_callback.callback = ffmpegCallBack;
    pContext->interrupt_callback.opaque = this;

    SDK_LOG_D("decodedFFmpegThread 开始解码");
    char *buf;
    int error_code = avformat_open_input(&pContext, url, NULL, NULL);
    if (error_code != 0) {
        SDK_LOG_D("avformat_open_input error code: %d , url: %s", error_code, url);
        // 打印错误日志
        if (av_strerror(error_code, buf, 1024) == 0) {
            SDK_LOG_D("avformat_open_input error code: %d , str: %s , url: %s", error_code, buf,
                      url);
        }
        exit = true;
        pthread_mutex_unlock(&initMutex);
        return;
    }
    if (avformat_find_stream_info(pContext, NULL) < 0) {
        SDK_LOG_D("avformat_find_stream_info error");
        exit = true;
        pthread_mutex_unlock(&initMutex);
        return;
    }
    SDK_LOG_D("pContext->nb_streams :%d ", pContext->nb_streams);
    for (int i = 0; i < (pContext->nb_streams); ++i) {
        if (pContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (pAudio == NULL) {
                SDK_LOG_D("pAudio == NULL ");
                pAudio = new TXAudio(playStatus, callJava,
                                     pContext->streams[i]->codecpar->sample_rate);
                pAudio->streamIndex = i;
                pAudio->codecpar = pContext->streams[i]->codecpar;
                pAudio->duration = pContext->duration / AV_TIME_BASE;
                pAudio->avRational = pContext->streams[i]->time_base;
            }
            SDK_LOG_D("pAudio != NULL ");
        }
    }
    // 获取解码器
    AVCodec *pCodec = avcodec_find_decoder(pAudio->codecpar->codec_id);
    if (!pCodec) {
        SDK_LOG_D("avcodec_find_decoder error");
        exit = true;
        pthread_mutex_unlock(&initMutex);
        return;
    }

    // 获取解码器上下文
    pAudio->pCodecContext = avcodec_alloc_context3(pCodec);
    if (!pAudio) {
        SDK_LOG_D("avcodec_alloc_context3 error");
        exit = true;
        pthread_mutex_unlock(&initMutex);
        return;
    }

    if (avcodec_parameters_to_context(pAudio->pCodecContext, pAudio->codecpar) < 0) {
        SDK_LOG_D("avcodec_parameters_to_context error");
        exit = true;
        pthread_mutex_unlock(&initMutex);
        return;
    }

    if (avcodec_open2(pAudio->pCodecContext, pCodec, 0) != 0) {
        SDK_LOG_D("avcodec_open2 error");
        exit = true;
        pthread_mutex_unlock(&initMutex);
        return;
    }
    this->callJava->onParpared(CHILD_THREAD);
    pthread_mutex_unlock(&initMutex);
}

void TXFFmpeg::start() {
    if (pAudio == NULL) {
        SDK_LOG_D("start error");
        return;
    }
    int count = 0;
    pAudio->play();
    while (playStatus != NULL && !playStatus->exit) {
        AVPacket *pPacket = av_packet_alloc();
        int readFrame = av_read_frame(pContext, pPacket);
        if (readFrame == 0) {
            if (pPacket->stream_index == pAudio->streamIndex) {
                count++;
                SDK_LOG_D("解码第 %d 帧 ", count);
                pAudio->queue->putAvpacket(pPacket);
//                av_packet_free(&pPacket);
//                av_free(pPacket);
//                pPacket = NULL;
            } else {
                av_packet_free(&pPacket);
                av_free(pPacket);
                pPacket = NULL;
            }
        } else {
            if (pPacket != NULL) {
                av_packet_free(&pPacket);
                av_free(pPacket);
                pPacket = NULL;
            }
            while (playStatus != NULL && !playStatus->exit) {
                SDK_LOG_D("释放 ");
                if (pAudio->queue->getQueueSize() > 0) {
                    continue;
                } else {
                    playStatus->exit = true;
                    break;
                }
            }

        }
    }
    exit = true;
//    while (pAudio->queue->getQueueSize() > 0) {
//        AVPacket *pPacket = av_packet_alloc();
//        pAudio->queue->getAvpacket(pPacket);
//        av_packet_free(&pPacket);
//        av_free(pPacket);
//        pPacket = NULL;
//    }
    SDK_LOG_D("解码完成");
}

void TXFFmpeg::resume() {
    if (pAudio != NULL) {
        pAudio->resume();
    }
}

void TXFFmpeg::pause() {
    if (pAudio != NULL) {
        pAudio->pause();
    }
}

void TXFFmpeg::release() {
    if (playStatus->exit) {
        return;
    }
    playStatus->exit = true;
    pthread_mutex_lock(&initMutex);
    int sleepCount = 0;
    while (!exit) {
        if (sleepCount > 1000) {
            exit = true;
        }
        SDK_LOG_D("wait ffmpeg %d", sleepCount);
        sleepCount++;
        av_usleep(1000 * 10);
    }
    if (pAudio != NULL) {
        pAudio->release();
        delete (pAudio);
        pAudio = NULL;
    }
    if (pContext != NULL) {
        avformat_close_input(&pContext);
        avformat_free_context(pContext);
        pContext = NULL;
    }
    if (callJava != NULL) {
        callJava = NULL;
    }
    if (playStatus != NULL) {
        playStatus = NULL;
    }
    pthread_mutex_unlock(&initMutex);
}

TXFFmpeg::~TXFFmpeg() {

}

