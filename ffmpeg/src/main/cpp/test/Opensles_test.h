//
// Created by yin13 on 2021/4/23.
//

#ifndef APPLE_OPENSLES_TEST_H
#define APPLE_OPENSLES_TEST_H

#include "stdio.h"
#include "../android_log.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "assert.h"
#include <malloc.h>

static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;
// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
static const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
static SLObjectItf pcmPlayObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLVolumeItf pcmPlayerVolume = NULL;

static FILE *pcmFile;
static uint8_t *outbuff;
static void *buffer;

// 获取pcm 数据
static int getPcmData(void **pcm) {
    int result = 0;
    while (!feof(pcmFile)) {
        // 返回 count 值
        result = fread(outbuff, 1, 44100 * 2 * 2, pcmFile);
        if (outbuff == NULL) {
            SDK_LOG_D("read end");
            break;
        } else {
            SDK_LOG_D("reading");
        }
        *pcm = outbuff;
        break;
    }
    return result;
}


// this callback handler is called every time a buffer finishes playing
static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    assert(bq == bqPlayerBufferQueue);
    assert(NULL == context);
    // for streaming playback, replace this test by logic to find and fill the next buffer
//    if (--nextCount > 0 && NULL != nextBuffer && 0 != nextSize) {
//        SLresult result;
//        // enqueue another buffer
//        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer, nextSize);
//        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
//        // which for this code example would indicate a programming error
//        if (SL_RESULT_SUCCESS != result) {
//            pthread_mutex_unlock(&audioEngineLock);
//        }
//        (void)result;
//    } else {
//        releaseResampleBuf();
//        pthread_mutex_unlock(&audioEngineLock);
//    }
    int nextSize = getPcmData(&buffer);
    SDK_LOG_D("读取 pcm 数据size= %d ", nextSize);
    assert(buffer);
    SLresult result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, buffer, nextSize);
}

class Opensles_test {
public:
    char *url;

public:

    Opensles_test(const char *url);

    ~Opensles_test();
};


#endif //APPLE_OPENSLES_TEST_H
