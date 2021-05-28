//
// Created by yin13 on 2021/4/17.
//

#include "TXFFmpeg.h"
#include "TXError.h"


TXFFmpeg::TXFFmpeg(TXCallJava *txCallJava, TXPlayStatus *txPlayStatus, const char *url) {
    this->callJava = txCallJava;
    this->url = url;
    this->playStatus = txPlayStatus;
    pthread_mutex_init(&initMutex, NULL);
    pthread_mutex_init(&seek_mutex, NULL);
}

TXFFmpeg::~TXFFmpeg() {
    SDK_LOG_D("release destroy");
    pthread_mutex_destroy(&initMutex);
    pthread_mutex_destroy(&seek_mutex);
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
            if (callJava != NULL) {
                callJava->onError(CHILD_THREAD, ERROR_CODE1, ERROR_MSG1);
            }
        }
        exit = true;
        pthread_mutex_unlock(&initMutex);
        return;
    }
    if (avformat_find_stream_info(pContext, NULL) < 0) {
        SDK_LOG_D("avformat_find_stream_info error");
        exit = true;
        pthread_mutex_unlock(&initMutex);
        if (callJava != NULL) {
            callJava->onError(CHILD_THREAD, ERROR_CODE2, ERROR_MSG2);
        }
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
                duration = pAudio->duration;
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
        if (callJava != NULL) {
            callJava->onError(CHILD_THREAD, ERROR_CODE3, ERROR_MSG3);
        }
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
        if (callJava != NULL) {
            callJava->onError(CHILD_THREAD, ERROR_CODE4, ERROR_MSG4);
        }
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

        if (playStatus->seek) {
            // 循环的地方 加睡眠,降低 CPU 使用率
            av_usleep(SLEEP_TIME);
            continue;
        }

        if (pAudio->queue->getQueueSize() > 40) {
            av_usleep(SLEEP_TIME);
            continue;
        }
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
                    av_usleep(SLEEP_TIME);
                    continue;
                } else {
                    playStatus->exit = true;
                    break;
                }
            }

        }
    }
    if (callJava != NULL) {
        callJava->onCallComplete(CHILD_THREAD);
    }
    exit = true;
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
    playStatus->exit = true;
    SDK_LOG_D("release start");
    pthread_mutex_lock(&initMutex);
    SDK_LOG_D("release start0101");
    int sleepCount = 0;
    while (!exit) {
        if (sleepCount > 100) {
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
    SDK_LOG_D("release pAudio");
    if (pContext != NULL) {
        avformat_close_input(&pContext);
        avformat_free_context(pContext);
        pContext = NULL;
    }
    SDK_LOG_D("release pContext");
    if (callJava != NULL) {
        delete (callJava);
        callJava = NULL;
    }
    SDK_LOG_D("release callJava");
    if (playStatus != NULL) {
        delete (playStatus);
        playStatus = NULL;
    }
    SDK_LOG_D("release playStatus");
    pthread_mutex_unlock(&initMutex);
    SDK_LOG_D("release----------");
}

void TXFFmpeg::setVolume(int percent) {
    if (pAudio != NULL) {
        pAudio->setVolume(percent);
    }
}

void TXFFmpeg::setSeek(int64_t seconds) {
    if (duration < 0) {
        SDK_LOG_D("duration %d", duration);
        return;
    }
    if (seconds >= 0 && seconds <= duration) {
        if (pAudio != NULL) {
            playStatus->seek = true;
            pAudio->queue->clearAvpacket();
            pAudio->clock = 0;
            pAudio->last_time = 0;
            pthread_mutex_lock(&seek_mutex);
            // 清空残留的 avframe
            avcodec_flush_buffers(pAudio->pCodecContext);
            int64_t rel = seconds * duration / 100 * AV_TIME_BASE;
            avformat_seek_file(pContext, -1, INT64_MIN, rel, INT64_MAX, 0);
            pthread_mutex_unlock(&seek_mutex);
            playStatus->seek = false;
        }
    }
}

void TXFFmpeg::setMute(int channel) {
    if (pAudio != NULL) {
        pAudio->setMute(channel);
    }
}

void TXFFmpeg::setPitch(float pitch) {
    if (pAudio != NULL) {
        pAudio->setPitch(pitch);
    }
}

void TXFFmpeg::setSpeed(float speed) {
    if (pAudio != NULL) {
        pAudio->setSpeed(speed);
    }
}

jint TXFFmpeg::getSampleRate() {
    if (pAudio != NULL) {
        return pAudio->sample_rate;
    }
    return 0;
}

void TXFFmpeg::startRecord() {
    if (pAudio != NULL) {
        pAudio->resumeRecord = true;
        pAudio->recordTime = 0.0f;
    }
}

void TXFFmpeg::stopRecord() {
    if (pAudio != NULL) {
        pAudio->resumeRecord = false;
        pAudio->recordTime = 0.0f;
    }
}

void TXFFmpeg::resumeRecord(bool isRecord) {
    if (pAudio != NULL) {
        pAudio->resumeRecord = isRecord;
    }
}

void TXFFmpeg::cutAudio(jint startTime, jint endTime, jboolean isShowPcm, const char *url) {
    if (pAudio != NULL) {
        SDK_LOG_D("startTime %d,endTime %d ", startTime, endTime);
        if (startTime > 0 && endTime > startTime && endTime <= (pAudio->duration)) {
            pAudio->startTime = startTime;
            pAudio->endTime = endTime;
            pAudio->isShowPcm = isShowPcm;
            FILE *cutFile = fopen(url, "w");
            pAudio->cutFile = cutFile;
        }
    }
}



