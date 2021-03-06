//
// Created by yin13 on 2021/4/17.
//
#include "TXCallJava.h"
#include "pthread.h"
#include "string.h"
#include "TXAudio.h"
#include "TXError.h"
#include "TXVideo.h"

#ifndef APPLE_TXFFMPEG_H
#define APPLE_TXFFMPEG_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
}

class TXFFmpeg {
public:
    TXCallJava *callJava = NULL;
    pthread_t decodeThread = NULL;
    const char *url = NULL;
    AVFormatContext *pContext = NULL;
    TXAudio *pAudio = NULL;
    TXVideo *pVideo = NULL;
    TXPlayStatus *playStatus = NULL;
    pthread_mutex_t initMutex;
    bool exit = false;
    int duration = 0;
    pthread_mutex_t seek_mutex;
    const AVBitStreamFilter *bsFilter = NULL;

public:
    TXFFmpeg(TXCallJava *txCallJava, TXPlayStatus *txPlayStatus, const char *url);

    ~TXFFmpeg();

    void parpared();

    // 解码器
    void decodedFFmpegThread();

    void start();

    void resume();

    void pause();

    void release();

    void setVolume(int percent);

    void setSeek(int64_t seconds);

    void setMute(int channel);

    void setPitch(float pitch);

    void setSpeed(float speed);

    void startRecord();

    void resumeRecord(bool resume);

    void stopRecord();

    jint getSampleRate();

    void cutAudio(jint startTime, jint endTime, jboolean isShowPcm, const char *url);

    int getCodecContext(AVCodecParameters *avCodecParameters, AVCodecContext **avCodecContext);
};


#endif //APPLE_TXFFMPEG_H
