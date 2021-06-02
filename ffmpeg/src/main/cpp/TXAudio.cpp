//
// Created by yin13 on 2021/4/17.
//

#include "TXAudio.h"

TXAudio::TXAudio(TXPlayStatus *txPlayStatus, TXCallJava *txCallJava, int sample_rate) {
    SDK_LOG_D("TXAudio ");
    this->playStatus = txPlayStatus;
    this->sample_rate = sample_rate;
    this->txCallJava = txCallJava;
    pthread_mutex_init(&pthreadMutex, NULL);

    queue = new TXQueue(playStatus);
    buffer = (uint8_t *) (av_malloc(sample_rate * 2 * 2));

    // soundTouch 初始化 设置采样率 声道数
    soundTouch = new SoundTouch();
    sampleBuffer = static_cast<SAMPLETYPE *>(malloc(sample_rate * 2 * 2));
    soundTouch->setSampleRate(sample_rate);
    soundTouch->setChannels(2);
    soundTouch->setPitch(pitchPercent);
    soundTouch->setTempo(speedPercent);

    // bufferqueue
    bufferQueue = new TXBufferQueue(txPlayStatus);
}

TXAudio::~TXAudio() {
    pthread_mutex_destroy(&pthreadMutex);
}

// 音频分包机制
void *bufferBack(void *data) {
    // 录音文件提取
    TXAudio *txAudio = static_cast<TXAudio *>(data);
    while (txAudio != NULL && txAudio->playStatus != NULL && !txAudio->playStatus->exit) {
        SDK_LOG_D("bufferBack");
        if (txAudio->resumeRecord) {
            if (txAudio->bufferQueue->getBufferSize() > 0) {
                TXPcmBean *txPcmBean = NULL;
                txAudio->bufferQueue->getBuffer(&txPcmBean);
                if (txPcmBean == NULL) {
                    continue;
                }
                SDK_LOG_D("txPcmBean buffer size is %d", txPcmBean->buffersize);
                // 不需要分包
                if (txPcmBean->buffersize < txAudio->defaultBufferSize) {
                    SDK_LOG_D("txPcmBean buffer 不需要分包");
                    if (txAudio->resumeRecord) {
                        txAudio->txCallJava->onCallOnPcmTAAc(CHILD_THREAD, txPcmBean->buffersize,
                                                             txPcmBean->buffer);
                    }
                } else {
                    // 需要分包处理
                    // 2021.05.27 分包后录音会有卡顿的声音，后续再去处理
                    int num = txPcmBean->buffersize / txAudio->defaultBufferSize;
                    int sub = txPcmBean->buffersize % txAudio->defaultBufferSize;
                    SDK_LOG_D("txPcmBean buffer 需要分包 num:%d,sub:%d", num, sub);
                    for (int i = 0; i < num; i++) {
                        if (txAudio->resumeRecord) {
                            char *buffer = static_cast<char *>(malloc(txAudio->defaultBufferSize));
                            memcmp(buffer,
                                   txPcmBean->buffer + (i * txAudio->defaultBufferSize),
                                   txAudio->defaultBufferSize);
                            txAudio->txCallJava->onCallOnPcmTAAc(CHILD_THREAD,
                                                                 txAudio->defaultBufferSize,
                                                                 buffer);
                            free(buffer);
                            buffer = NULL;
                        }
                    }
                    if (sub > 0) {
                        SDK_LOG_D("txPcmBean buffer 需要分包2 num:%d,sub:%d", num, sub);
                        if (txAudio->resumeRecord) {
                            char *buffer = static_cast<char *>(malloc(sub));
                            memcpy(buffer, txPcmBean->buffer + num * txAudio->defaultBufferSize,
                                   sub);
                            txAudio->txCallJava->onCallOnPcmTAAc(CHILD_THREAD,
                                                                 sub, buffer);
                            free(buffer);
                            buffer = NULL;
                        }
                    }
                }
                delete (txPcmBean);
                txPcmBean = NULL;
            }
        }
    }
    pthread_exit(&(txAudio->pthreadBuffer));
    SDK_LOG_D("txPcmBean buffer exit");
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
    pthread_create(&pthreadBuffer, NULL, bufferBack, this);
}

FILE *outFile = fopen("/data/data/com.taxiao.cn.apple/cache/test2.pcm", "w");

int TXAudio::resampleAudio(void **pcmbuffer) {
    SDK_LOG_D("resampleAudio");
    while (playStatus != NULL && !playStatus->exit) {
        if (queue->getQueueSize() == 0) {
            SDK_LOG_D("resampleAudio 加载");
            av_usleep(SLEEP_TIME);
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
        // .ape 文件一个 AVPacket 会有多个 AVFrame ,需要多次 avcodec_receive_frame 取出
        pthread_mutex_lock(&pthreadMutex);
        if (readFrameFinished) {
            SDK_LOG_D("resampleAudio 获取数据");
            avPacket = av_packet_alloc();
            if (queue->getAvpacket(avPacket) != 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&pthreadMutex);
                continue;
            }
            ret = avcodec_send_packet(pCodecContext, avPacket);
            if (ret != 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&pthreadMutex);
                continue;
            }
        }
        // 发送解码器，成功会返回一个avFrame
        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(pCodecContext, avFrame);
        if (ret == 0) {
            readFrameFinished = false;
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
                readFrameFinished = true;
                pthread_mutex_unlock(&pthreadMutex);
                continue;
            }

            nb = swr_convert(swrContext, &buffer, avFrame->nb_samples,
                             (const uint8_t **) (avFrame->data), avFrame->nb_samples);
            // 声道
            int channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

            data_size = nb * channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            // 帧数* （时间/帧数）
            now_time = avFrame->pts * av_q2d(avRational);
            if (now_time < clock) {
                now_time = clock;
            }
            clock = now_time;

            // 将buffer 指向pcmbuffer soundtouch 用
            if (pcmbuffer != NULL) {
                *pcmbuffer = buffer;
            }
            // 写入文件
//            fwrite(buffer, 1, data_size, outFile);
            SDK_LOG_D("resampleAudio 写入文件");

            // 时间 返回
//            if (data_size > 0) {
//                SDK_LOG_D("bqPlayerCallback %d ", data_size);
//                // buffer 理论上播放需要的时间
//                clock += data_size / ((double) (sample_rate * 2 * 2));
//                if (clock - last_time >= 0.1) {
//                    last_time = clock;
//                    txCallJava->onTimeInfo(CHILD_THREAD, clock, duration);
//                }
//            }

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swrContext);
            swrContext = NULL;
            pthread_mutex_unlock(&pthreadMutex);
            break;
        } else {
            readFrameFinished = true;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            pthread_mutex_unlock(&pthreadMutex);
            continue;
        }
    }
    return data_size;
}

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    TXAudio *txAudio = (TXAudio *) context;
    if (txAudio != NULL) {
        if (txAudio->playStatus->exit) {
            return;
        }
        int bufferSize = 0;
        if (USE_SOUND_TOUCH) {
            // 使用 soundtouch
            bufferSize = txAudio->getSoundTouchData();
        } else {
            bufferSize = txAudio->resampleAudio(NULL);
        }
        if (bufferSize > 0) {
            SDK_LOG_D("bqPlayerCallback %d ", bufferSize);
            // buffer 理论上播放需要的时间
            txAudio->clock += bufferSize / ((double) (txAudio->sample_rate * 2 * 2));
            if (txAudio->clock - txAudio->last_time >= 0.1) {
                txAudio->last_time = txAudio->clock;
                txAudio->txCallJava
                        ->onTimeInfo(CHILD_THREAD, txAudio->clock, txAudio->duration);
                SDK_LOG_D("currenttime %d , totalTime %d ", (int) txAudio->clock,
                          txAudio->duration);
            }
            if (USE_SOUND_TOUCH) {
                // 使用 soundtouch
                (*txAudio->bqPlayerBufferQueue)->Enqueue(txAudio->bqPlayerBufferQueue,
                                                         (char *) txAudio->sampleBuffer,
                                                         bufferSize * 2 * 2);

                txAudio->txCallJava->onCallValumeDB(CHILD_THREAD, txAudio->getPcmDB(
                        reinterpret_cast<char *>(txAudio->sampleBuffer), bufferSize * 2 * 2));

                // media codec 设置 KEY_MAX_INPUT_SIZE，需要做分包处理
                if (txAudio->resumeRecord) {
                    txAudio->bufferQueue->putBuffer(txAudio->sampleBuffer, bufferSize * 2 * 2);
                }

                if (txAudio->resumeRecord) {
//                    txAudio->txCallJava->onCallOnPcmTAAc(CHILD_THREAD, bufferSize * 2 * 2,
//                                                         txAudio->sampleBuffer);
                    float time = (bufferSize * 2 * 2 * 1.0f) / (2 * 2 * (txAudio->sample_rate));
                    (txAudio->recordTime) += time;
                    txAudio->txCallJava->onCallOnRecordTime(CHILD_THREAD, txAudio->recordTime);
                    SDK_LOG_D("totalTime: %f , currentTime: %f ", txAudio->recordTime, time);
                }
                // 时间需要强转
                int currentTime = (int) txAudio->clock;
                if (txAudio->startTime > 0 && txAudio->endTime > 0 && txAudio->endTime > txAudio
                        ->startTime && (currentTime >= txAudio->startTime) &&
                    (currentTime <= txAudio->endTime)) {
                    // 存储剪切文件
                    if (txAudio->cutFile != NULL) {
                        fwrite(txAudio->sampleBuffer, 1, bufferSize * 2 * 2, txAudio->cutFile);
                    }
                    if (txAudio->isShowPcm) {
                        SDK_LOG_D("write currenttime %d , totalTime %d ", currentTime,
                                  txAudio->duration);
                        txAudio->txCallJava->onCallOnCutAudio(CHILD_THREAD, txAudio->sample_rate,
                                                              bufferSize * 2 * 2,
                                                              txAudio->sampleBuffer);
                    }
                }
            } else {
                (*txAudio->bqPlayerBufferQueue)->Enqueue(txAudio->bqPlayerBufferQueue,
                                                         (char *) txAudio->buffer,
                                                         bufferSize);
                // 回调分贝值
                txAudio->txCallJava->onCallValumeDB(CHILD_THREAD, txAudio->getPcmDB(
                        reinterpret_cast<char *>(txAudio->buffer), bufferSize * 2 * 2));

                // media codec 设置 KEY_MAX_INPUT_SIZE，需要做分包处理
                if (txAudio->resumeRecord) {
                    txAudio->bufferQueue->putBuffer(reinterpret_cast<SAMPLETYPE *>(txAudio->buffer),
                                                    bufferSize * 2 * 2);
                }

//                if (txAudio->resumeRecord) {
//                    txAudio->txCallJava->onCallOnPcmTAAc(CHILD_THREAD, bufferSize * 2 * 2,
//                                                         txAudio->buffer);
//                    float time = (bufferSize * 2 * 2 * 1.0f) / (2 * 2 * (txAudio->sample_rate));
//                    (txAudio->recordTime) += time;
//                    txAudio->txCallJava->onCallOnRecordTime(CHILD_THREAD, txAudio->recordTime);
//                    SDK_LOG_D("totalTime: %f , currentTime: %f ", txAudio->recordTime, time);
//                }
                // 时间需要强转
                int currentTime = (int) txAudio->clock;
                if (txAudio->startTime > 0 && txAudio->endTime > 0 && txAudio->endTime > txAudio
                        ->startTime && (currentTime >= txAudio->startTime) &&
                    (currentTime <= txAudio->endTime)) {
                    // 存储剪切文件
                    if (txAudio->cutFile != NULL) {
                        fwrite(txAudio->buffer, 1, bufferSize * 2 * 2, txAudio->cutFile);
                    }
                    if (txAudio->isShowPcm) {
                        SDK_LOG_D("write currenttime %d , totalTime %d ", currentTime,
                                  txAudio->duration);
                        txAudio->txCallJava->onCallOnCutAudio(CHILD_THREAD, txAudio->sample_rate,
                                                              bufferSize * 2 * 2,
                                                              txAudio->buffer);
                    }
                }
            }
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

    // 7.创建播放器 SLDataLocator_AndroidBufferQueue 会报错 配置 PCM 格式信息
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
    *
    * SL_IID_PLAYBACKRATE 解决切换音乐时卡顿
    */
    const SLInterfaceID audio_ids[5] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME,
                                        SL_IID_MUTESOLO, SL_IID_PLAYBACKRATE};
    const SLboolean audio_req[5] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
                                    SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
//    const SLInterfaceID audio_ids[1] = {SL_IID_BUFFERQUEUE};
//    const SLboolean audio_req[1] = {SL_BOOLEAN_TRUE};

    // create audio player:
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayObject, &slDataSource,
                                                &audioSnk, 5, audio_ids, audio_req);
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

    // get volume
    result = (*pcmPlayObject)->GetInterface(pcmPlayObject, SL_IID_VOLUME, &pcmPlayVolume);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the mute/solo interface
    result = (*pcmPlayObject)->GetInterface(pcmPlayObject, SL_IID_MUTESOLO, &fdPlayerMuteSolo);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the buffer queue interface
    result = (*pcmPlayObject)->GetInterface(pcmPlayObject, SL_IID_BUFFERQUEUE,
                                            &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    SDK_LOG_D("get the buffer");

    setVolume(volumePercent);
    setMute(muteChannel);

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
    if (bqPlayerPlay != NULL) {
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    }
}

// 暂停播放
void TXAudio::pause() {
    if (bqPlayerPlay != NULL) {
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
    }
}

// 停止播放
void TXAudio::stop() {
    recordTime = 0.0f;
    if (bqPlayerPlay != NULL) {
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
    }
}

void TXAudio::release() {
    stop();
    if (queue != NULL) {
        delete (queue);
        queue = NULL;
    }
    if (pcmPlayObject != NULL) {
        (*pcmPlayObject)->Destroy(pcmPlayObject);
        pcmPlayObject = NULL;
    }
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
    }
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
    }
    if (pcmPlayVolume != NULL) {
        pcmPlayVolume = NULL;
    }
    if (fdPlayerMuteSolo != NULL) {
        fdPlayerMuteSolo = NULL;
    }
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
    if (soundTouch != NULL) {
        soundTouch->clear();
        delete (soundTouch);
        soundTouch = NULL;
    }
    if (sampleBuffer != NULL) {
        free(sampleBuffer);
        sampleBuffer = NULL;
    }
    if (out_buffer != NULL) {
        out_buffer = NULL;
    }
    if (pCodecContext != NULL) {
        avcodec_close(pCodecContext);
        avcodec_free_context(&pCodecContext);
        pCodecContext = NULL;
    }
    if (playStatus != NULL) {
        playStatus = NULL;
    }
    if (txCallJava != NULL) {
        txCallJava = NULL;
    }
    startTime = 0;
    endTime = 0;
    isShowPcm = false;
    if (cutFile != NULL) {
        cutFile = NULL;
    }
    if (bufferQueue != NULL) {
        bufferQueue->noticeThread();
        pthread_join(pthreadBuffer, NULL);
        bufferQueue->release();
        delete (bufferQueue);
        bufferQueue = NULL;
    }
}

// 0是最大值 -5000是最小值
void TXAudio::setVolume(int percent) {
    volumePercent = percent;
    if (pcmPlayVolume != NULL) {
        if (percent > 30) {
            (*pcmPlayVolume)->SetVolumeLevel(pcmPlayVolume, (100 - percent) * -20);
        } else if (percent > 25) {
            (*pcmPlayVolume)->SetVolumeLevel(pcmPlayVolume, (100 - percent) * -22);
        } else if (percent > 20) {
            (*pcmPlayVolume)->SetVolumeLevel(pcmPlayVolume, (100 - percent) * -25);
        } else if (percent > 15) {
            (*pcmPlayVolume)->SetVolumeLevel(pcmPlayVolume, (100 - percent) * -28);
        } else if (percent > 10) {
            (*pcmPlayVolume)->SetVolumeLevel(pcmPlayVolume, (100 - percent) * -30);
        } else if (percent > 5) {
            (*pcmPlayVolume)->SetVolumeLevel(pcmPlayVolume, (100 - percent) * -34);
        } else if (percent > 3) {
            (*pcmPlayVolume)->SetVolumeLevel(pcmPlayVolume, (100 - percent) * -37);
        } else if (percent > 0) {
            (*pcmPlayVolume)->SetVolumeLevel(pcmPlayVolume, (100 - percent) * -40);
        } else {
            (*pcmPlayVolume)->SetVolumeLevel(pcmPlayVolume, (100 - percent) * -100);
        }
    }
}

// 设置声道 0右声道 1左声道 2立体声
void TXAudio::setMute(int channel) {
    muteChannel = channel;
    if (fdPlayerMuteSolo != NULL) {
        if (channel == 0) {
            (*fdPlayerMuteSolo)->SetChannelMute(fdPlayerMuteSolo, CHANNEL_LEFT, false);
            (*fdPlayerMuteSolo)->SetChannelMute(fdPlayerMuteSolo, CHANNEL_RIGHT, true);
        } else if (channel == 1) {
            (*fdPlayerMuteSolo)->SetChannelMute(fdPlayerMuteSolo, CHANNEL_LEFT, true);
            (*fdPlayerMuteSolo)->SetChannelMute(fdPlayerMuteSolo, CHANNEL_RIGHT, false);
        } else if (channel == 2) {
            (*fdPlayerMuteSolo)->SetChannelMute(fdPlayerMuteSolo, CHANNEL_LEFT, false);
            (*fdPlayerMuteSolo)->SetChannelMute(fdPlayerMuteSolo, CHANNEL_RIGHT, false);
        }
    }
}

// soundTouch 变速 变调 获取采样个数
int TXAudio::getSoundTouchData() {
    while (playStatus != NULL && !playStatus->exit) {
        out_buffer = NULL;
        SDK_LOG_D("------------------------1");
        if (finish) {
            finish = false;
            // 只计算一次 sampleBuffer
            SDK_LOG_D("------------------------2");
            data_size = resampleAudio(reinterpret_cast<void **>(&out_buffer));
            SDK_LOG_D("------------------------21");
            if (data_size > 0) {
                for (int i = 0; i < data_size / 2 + 1; i++) {
                    // FFmpeg 解出的数据是8bit, soundtouch 最低支持是16bit,
                    if (out_buffer != NULL) {
                        sampleBuffer[i] = (out_buffer[i * 2] | (out_buffer[i * 2 + 1] << 8));
                    }
                }
                if (soundTouch != NULL) {
                    soundTouch->putSamples(sampleBuffer, nb);
                    // /4  两个声道 16位
                    num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
                    SDK_LOG_D("------------------------3");
                    SDK_LOG_D("putSamples");
                }
            } else {
                SDK_LOG_D("------------------------4");
                if (soundTouch != NULL) {
                    soundTouch->flush();
                }
            }
        }
        if (num == 0) {
            finish = true;
            continue;
        } else {
            if (out_buffer == NULL) {
                SDK_LOG_D("------------------------5");
                if (soundTouch != NULL) {
                    num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
                }
                SDK_LOG_D("receiveSamples");
                if (num == 0) {
                    finish = true;
                    continue;
                }
            }
            return num;
        }
    }
    return 0;
}

void TXAudio::setPitch(float pitch) {
    pitchPercent = pitch;
    if (soundTouch != NULL) {
        soundTouch->setPitch(pitch);
    }
}

void TXAudio::setSpeed(float speed) {
    speedPercent = speed;
    if (soundTouch != NULL) {
        soundTouch->setTempo(speed);
    }
}

int TXAudio::getPcmDB(char *pcmcta, size_t pcmsize) {
    int db = 0;
    short int pervalve = 0;
    double sum = 0;
    for (int i = 0; i < pcmsize; i += 2) {
        memcpy(&pervalve, pcmcta, 2);
        sum += abs(pervalve);
    }
    sum = sum / (pcmsize / 2);
    if (sum > 0) {
        db = 20.0 * log10(sum);
    }
    SDK_LOG_D("getPcmdb %d ", db);
    return db;
}
