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
#include <libavutil/rational.h>
#include <libavutil/time.h>
}

#include "soundtouch/source/SoundTouch.h"
#include "TXBufferQueue.h"

using namespace soundtouch;

#ifndef APPLE_TXAUDIO_H
#define APPLE_TXAUDIO_H

#define CHANNEL_RIGHT 0
#define CHANNEL_LEFT 1

// true 使用 soundtouch
#define USE_SOUND_TOUCH true
#define SLEEP_TIME 1000*100

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
    bool readFrameFinished = true;

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
    SLVolumeItf pcmPlayVolume = NULL;
    SLMuteSoloItf fdPlayerMuteSolo = NULL; // 声道
    int volumePercent = 100;
    int muteChannel = CHANNEL_LEFT;
    int pitchPercent = 1.0;
    int speedPercent = 1.0;

    // duration
    int duration = 0; // 总时间
    AVRational avRational; // 总时间/总帧数
    double now_time = 0.0; // 现在时间
    double clock = 0.0; // 当前时间
    double last_time = 0.0;

    // soundtouch
    SoundTouch *soundTouch = NULL;
    SAMPLETYPE *sampleBuffer = NULL;
    uint8_t *out_buffer = NULL;
    bool finish = true;
    int nb = 0;
    int num = 0;

    // record 默认 false
    bool resumeRecord = false;
    float recordTime = 0.0f;
    int startTime = 0;
    int endTime = 0;
    bool isShowPcm = false;
    FILE *cutFile = NULL;

    TXBufferQueue *bufferQueue = NULL;
    pthread_t pthreadBuffer;
    int defaultBufferSize = 4096;

    pthread_mutex_t pthreadMutex;

public:
    TXAudio(TXPlayStatus *txPlayStatus, TXCallJava *txCallJava, int sample_rate);

    ~TXAudio();

    void play();

    int resampleAudio(void **buffer);

    void initOpenSLES();

    void resume();

    void pause();

    void stop();

    void release();

    int getCurrentSimpleRate(int sample_rate);

    void setVolume(int percent);

    void setMute(int channel);

    int getSoundTouchData();

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getPcmDB(char *pcmcta, size_t pcmsize);

};


#endif //APPLE_TXAUDIO_H
