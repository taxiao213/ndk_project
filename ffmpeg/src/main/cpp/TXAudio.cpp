//
// Created by yin13 on 2021/4/17.
//

#include "TXAudio.h"

TXAudio::TXAudio(TXPlayStatus *txPlayStatus, TXCallJava *txCallJava, int sample_rate) {
    SDK_LOG_D("TXAudio ");
    this->playStatus = txPlayStatus;
    this->sample_rate = sample_rate;
    this->txCallJava = txCallJava;
    queue = new TXQueue(playStatus);
    buffer = (uint8_t *) (av_malloc(sample_rate * 2 * 2));
}

// 重采样
void *decodePlay(void *data) {
    SDK_LOG_D("decodePlay");
    TXAudio *txAudio = (TXAudio *) (data);
//    txAudio->resampleAudio();
    txAudio->initOpenSLES();
    pthread_exit(&(txAudio->p_thread));
}

void TXAudio::play() {
    SDK_LOG_D("play");
    pthread_create(&p_thread, NULL, decodePlay, this);
}

//FILE *outFile = fopen("/data/data/com.taxiao.cn.apple/cache/test.pcm", "w");

int TXAudio::resampleAudio() {
    SDK_LOG_D("resampleAudio");
    while (playStatus != NULL && !playStatus->exit) {
        if (queue->getQueueSize() == 0) {
            SDK_LOG_D("resampleAudio 加载");
            if (!playStatus->load) {
                playStatus->load = true;
                txCallJava->onLoad(CHILD_THREAD, true);
            }
            continue;
        } else {
            SDK_LOG_D("resampleAudio 播放");
            if (playStatus->load) {
                playStatus->load = false;
                txCallJava->onLoad(CHILD_THREAD, false);
            }
        }

        SDK_LOG_D("resampleAudio 获取数据");
        avPacket = av_packet_alloc();
        if (queue->getAvpacket(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        ret = avcodec_send_packet(pCodecContext, avPacket);
        if (ret != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        // 发送解码器，成功会返回一个avFrame
        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(pCodecContext, avFrame);
        if (ret == 0) {
            // 0 成功 声道布局
            if (avFrame->channels > 0 && avFrame->channel_layout == 0) {
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }
            // 重采样
            SwrContext *swrContext = NULL;
            swrContext = swr_alloc_set_opts(NULL,
                                            AV_CH_LAYOUT_STEREO,// 声道位置
                                            AV_SAMPLE_FMT_S16,// 输出的采样数
                                            avFrame->sample_rate,
                                            avFrame->channel_layout,
                                            (AVSampleFormat) avFrame->format,
                                            avFrame->sample_rate,
                                            NULL,
                                            NULL);
            if (!swrContext || swr_init(swrContext) < 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                if (swrContext != NULL) {
                    swr_free(&swrContext);
                    swrContext = NULL;
                }
                continue;
            }

            int convert = swr_convert(swrContext, &buffer, avFrame->nb_samples,
                                      (const uint8_t **) (avFrame->data), avFrame->nb_samples);
            // 声道
            int channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

            data_size = convert * channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            // 帧数* （时间/帧数）
            now_time = avFrame->pts * av_q2d(avRational);
            if (now_time < clock) {
                now_time = clock;
            }
            clock = now_time;

            // 写入文件
//            fwrite(buffer, 1, data_size, outFile);
            SDK_LOG_D("resampleAudio 写入文件");

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swrContext);
            swrContext = NULL;
            break;
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }
    }
    return data_size;
}

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    TXAudio *txAudio = (TXAudio *) context;
    if (txAudio != NULL) {
        int bufferSize = txAudio->resampleAudio();
        if (bufferSize > 0) {
            SDK_LOG_D("bqPlayerCallback %d ", bufferSize);
            // buffer 理论上播放需要的时间
            txAudio->clock += bufferSize / ((double) (txAudio->sample_rate * 2 * 2));
            if (txAudio->clock - txAudio->last_time >= 0.1) {
                txAudio->last_time = txAudio->clock;
                txAudio->txCallJava
                        ->onTimeInfo(CHILD_THREAD, txAudio->clock, txAudio->duration);
            }
            (*txAudio->bqPlayerBufferQueue)->Enqueue(txAudio->bqPlayerBufferQueue,
                                                     (char *) txAudio->buffer,
                                                     bufferSize);
        }
    }
}

int TXAudio::getCurrentSimpleRate(int sample_rate) {
    int rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

void TXAudio::initOpenSLES() {
    SDK_LOG_D("intOpenSLES");
    SLresult result;
    // 1.create engine
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // 2.realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    SDK_LOG_D("realize the engine");

    // 3.get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    SDK_LOG_D("get the engine interface");

    // 4.create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    SDK_LOG_D("create output mix");

    // 5.realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    SDK_LOG_D("realize the output mix");

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
            static_cast<SLuint32>(getCurrentSimpleRate(sample_rate)),//输入的采样率，44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//输入的采样位数，16bit
            SL_PCMSAMPLEFORMAT_FIXED_16,//容器大小，同上
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//声道标记，这里使用左前声道和右前声道
            SL_BYTEORDER_LITTLEENDIAN//输入的字节序,小端
    };
    SLDataSource slDataSource = {&androidBufferQueue, &pcm};
    SLDataSink audioSnk = {&outputMix, NULL};
    SDK_LOG_D("创建播放器");

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
    SDK_LOG_D("realize the player");

    // get the play interface
    result = (*pcmPlayObject)->GetInterface(pcmPlayObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the buffer queue interface
    result = (*pcmPlayObject)->GetInterface(pcmPlayObject, SL_IID_BUFFERQUEUE,
                                            &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    SDK_LOG_D("get the buffer");

    // 设置回调函数
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    SDK_LOG_D("设置回调函数");

    // 获取音量接口
//    result = (*pcmPlayObject)->GetInterface(pcmPlayObject, SL_IID_VOLUME, &pcmPlayerVolume);
//    assert(SL_RESULT_SUCCESS == result);
//    (void) result;

    // 设置播放状态
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    SDK_LOG_D("设置播放状态");

    bqPlayerCallback(bqPlayerBufferQueue, this);
    SDK_LOG_D("video complate");

}

// 恢复播放
void TXAudio::resume() {
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
}

// 暂停播放
void TXAudio::pause() {
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
}
