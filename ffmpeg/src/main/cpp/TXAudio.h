//
// Created by yin13 on 2021/4/17.
//
#include "TXQueue.h"
#include "TXPlayStatus.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "assert.h"
#include "TXCallJava.h"
#include <malloc.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#ifndef APPLE_TXAUDIO_H
#define APPLE_TXAUDIO_H

class TXAudio {
public:
    int streamIndex = -1;
    AVCodecParameters *codecpar = NULL;
    AVCodecContext *pCodecContext = NULL;
    TXQueue *queue = NULL;
    TXPlayStatus *playStatus = NULL;
    pthread_t p_thread = NULL;
    AVPacket *avPacket = NULL;
    int ret = -1;
    AVFrame *avFrame = NULL;
    uint8_t *buffer = NULL;
    int data_size = 0;
    int sample_rate = 0;
    TXCallJava *txCallJava = NULL;

    // OpenSLES
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;
    // output mix interfaces
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    SLObjectItf pcmPlayObject = NULL;
    SLPlayItf bqPlayerPlay = NULL;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = NULL;

public:
    TXAudio(TXPlayStatus *txPlayStatus, TXCallJava *txCallJava, int sample_rate);

    ~TXAudio();

    void play();

    int resampleAudio();

    void initOpenSLES();

    void resume();

    void pause();

    int getCurrentSimpleRate(int sample_rate);
};


#endif //APPLE_TXAUDIO_H
