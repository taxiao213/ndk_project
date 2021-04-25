//
// Created by yin13 on 2021/4/17.
//

#include "TXAudio.h"

TXAudio::TXAudio(TXPlayStatus *txPlayStatus) {
    SDK_LOG_D("TXAudio ");
    this->playStatus = txPlayStatus;
    queue = new TXQueue(playStatus);
    buffer = (uint8_t *) (av_malloc(44100 * 2 * 2));
}

// 重采样
void *decodePlay(void *data) {
    SDK_LOG_D("decodePlay");
    TXAudio *txAudio = (TXAudio *) (data);
    txAudio->resampleAudio();
    pthread_exit(&(txAudio->p_thread));
}

void TXAudio::play() {
    SDK_LOG_D("play");
    pthread_create(&p_thread, NULL, decodePlay, this);
}

FILE *outFile = fopen("/data/data/com.taxiao.cn.apple/cache/test.pcm", "w");

int TXAudio::resampleAudio() {
    SDK_LOG_D("resampleAudio");
    int size = 0;
    while (playStatus != NULL && !playStatus->exit) {
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

            // 写入文件
            fwrite(buffer, 1, data_size, outFile);
            SDK_LOG_D("resampleAudio 写入文件");

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swrContext);
            swrContext = NULL;

        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
        }
    }


    return size;
}
