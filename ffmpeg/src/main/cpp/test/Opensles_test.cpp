//
// Created by yin13 on 2021/4/23.
//

#include "Opensles_test.h"

Opensles_test::Opensles_test(const char *url) {
    pcmFile = fopen(url, "rw");
    assert(pcmFile);
    // 16位  双声道2
    outbuff = (uint8_t *) (malloc(44100 * 2 * 2));

    SLresult result;
    // 1.create engine
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // 2.realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // 3.get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // 4.create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // 5.realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // 6. // get the environmental reverb interface
    //    // this could fail if the environmental reverb effect is not available,
    //    // either because the feature is not present, excessive CPU load, or
    //    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void) result;
    }

    // configure audio sink
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};

    // 7.创建播放器 SLDataLocator_AndroidBufferQueue 会报错
    SLDataLocator_AndroidSimpleBufferQueue androidBufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//输入的音频格式,PCM
            2,//输入的声道数，2(立体声)
            SL_SAMPLINGRATE_44_1,//输入的采样率，44100hz44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//输入的采样位数，16bit
            SL_PCMSAMPLEFORMAT_FIXED_16,//容器大小，同上
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//声道标记，这里使用左前声道和右前声道
            SL_BYTEORDER_LITTLEENDIAN//输入的字节序,小端
    };
    SLDataSource slDataSource = {&androidBufferQueue, &pcm};
    SLDataSink audioSnk = {&outputMix, NULL};

    /*
    * create audio player:
    * fast audio does not support when SL_IID_EFFECTSEND is required, skip it
    * for fast audio case
    */
    const SLInterfaceID audio_ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean audio_req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
//    const SLInterfaceID audio_ids[1] = {SL_IID_BUFFERQUEUE};
//    const SLboolean audio_req[1] = {SL_BOOLEAN_TRUE};

    // create audio player:
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayObject, &slDataSource,
                                                &audioSnk, 3, audio_ids, audio_req);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // realize the player
    result = (*pcmPlayObject)->Realize(pcmPlayObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the play interface
    result = (*pcmPlayObject)->GetInterface(pcmPlayObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the buffer queue interface
    result = (*pcmPlayObject)->GetInterface(pcmPlayObject, SL_IID_BUFFERQUEUE,
                                            &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // 设置回调函数
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // 获取音量接口
//    result = (*pcmPlayObject)->GetInterface(pcmPlayObject, SL_IID_VOLUME, &pcmPlayerVolume);
//    assert(SL_RESULT_SUCCESS == result);
//    (void) result;

    // 设置播放状态
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    bqPlayerCallback(bqPlayerBufferQueue, NULL);
    SDK_LOG_D("video complate");
}


