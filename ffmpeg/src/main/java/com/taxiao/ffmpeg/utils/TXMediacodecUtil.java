package com.taxiao.ffmpeg.utils;

import android.media.MediaCodec;
import android.media.MediaFormat;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * mediacodec 将 PCM 数据存储为 AAC 文件 实现录音
 * Created by hanqq on 2021/5/22
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class TXMediacodecUtil {
    private static final String TAG = TXMediacodecUtil.class.getSimpleName();

    private static volatile TXMediacodecUtil mMediaCodecUtils;
    private HandlerThread handlerThread;
    private AudioRunnable audioRunnable;
    private MediaCodec mediaCodec;
    private MediaCodec.BufferInfo info;
    private int mSampleRate;
    private int perpcmsize = 0;
    private byte[] outByteBuffer;
    private File mOutfile;
    private FileOutputStream outputStream;
    private MediaFormat mediaFormat;

    private TXMediacodecUtil() {
    }

    public static TXMediacodecUtil getInstance() {
        if (mMediaCodecUtils == null) {
            synchronized (TXMediacodecUtil.class) {
                if (mMediaCodecUtils == null) {
                    mMediaCodecUtils = new TXMediacodecUtil();
                }
            }
        }
        return mMediaCodecUtils;
    }

    public void createAudioCodec(int sampleRate, File outfile) {
        this.mSampleRate = sampleRate;
        this.mOutfile = outfile;
        handlerThread = new HandlerThread("audio_codec");
        handlerThread.start();
        Handler handler = new Handler(handlerThread.getLooper());
        audioRunnable = new AudioRunnable(sampleRate);
        handler.post(audioRunnable);
    }

    public void release() {
        Log.d(TAG, " mediacode destroy");
        try {
            if (handlerThread != null) {
                handlerThread.getLooper().quit();
                handlerThread.quitSafely();
                handlerThread = null;
            }
            if (audioRunnable != null) {
                audioRunnable = null;
            }
            if (mediaCodec != null) {
                mediaCodec.stop();
                mediaCodec.release();
                mediaCodec = null;
            }
            if (mediaFormat != null) {
                mediaFormat = null;
            }
            if (outputStream != null) {
                outputStream.close();
                outputStream = null;
            }
            if (info != null) {
                info = null;
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (outputStream != null) {
                try {
                    outputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                outputStream = null;
            }
        }
    }

    public class AudioRunnable implements Runnable {
        private int sampleRate;

        public AudioRunnable(int sampleRate) {
            this.sampleRate = sampleRate;
        }

        @Override
        public void run() {
            try {
                initMediacodec(sampleRate);
            } catch (Exception e) {
                release();
                e.printStackTrace();
            }
        }
    }

    private void initMediacodec(int sampleRate) throws IOException {
        mSampleRate = getADTSsamplerate(sampleRate);
        mediaFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, sampleRate, TXConstant.CHANNEL_COUNT);
        mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, 96000);
        // 码流压缩
        mediaFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, TXConstant.MEDIA_CODEC_PROFILE);
        // 决定输入的 encodePcmTAAc  byte 分包处理会有卡顿的声音，后续再去处理
        mediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 4096);
        mediaCodec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
        info = new MediaCodec.BufferInfo();
        mediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        outputStream = new FileOutputStream(mOutfile);
        mediaCodec.start();
    }

    /**
     * 同步模式
     * 使用 MediaCodec 将 PCM 数据存储为 AAC 文件
     * 问题: 当使用 soundTouch 时，如果不是在正常倍速打开录音 会有杂音
     */
    public void encodePcmTAAc(int size, byte[] buffer) {
        if (buffer != null && buffer.length > 0 && mediaCodec != null) {
            Log.d(TAG, " encodePcmTAAc size: " + size + " buffer size : " + buffer.length);
            try {
                // 从队列取出
                int inputBufferIndex = mediaCodec.dequeueInputBuffer(0);
                if (inputBufferIndex >= 0) {
                    // 获取到 input buffer
                    ByteBuffer inputBuffer = mediaCodec.getInputBuffers()[inputBufferIndex];
                    inputBuffer.clear();
                    inputBuffer.put(buffer);
                    // 将 input buffer 替换 再放入队列
                    mediaCodec.queueInputBuffer(inputBufferIndex, 0, size, 0, 0);
                }
                int outPutBufferIndex = mediaCodec.dequeueOutputBuffer(info, 0);
                Log.d(TAG, " encodePcmTAAc inputBufferIndex : " + inputBufferIndex + " outPutBufferIndex: " + outPutBufferIndex);
                while (outPutBufferIndex >= 0) {
                    Log.d(TAG, " encodePcmTAAc outPutBufferIndex2: " + outPutBufferIndex);
                    perpcmsize = info.size + 7;
                    outByteBuffer = new byte[perpcmsize];
                    ByteBuffer outputBuffer = mediaCodec.getOutputBuffers()[outPutBufferIndex];
                    // position：buffer当前所在的操作位置
                    // limit：buffer最大的操作位置
                    // capacity：buffer的最大长度
                    outputBuffer.position(info.offset);
                    outputBuffer.limit(info.offset + info.size);

                    addADtsHeader(outByteBuffer, perpcmsize, mSampleRate);

                    outputBuffer.get(outByteBuffer, 7, info.size);
                    outputBuffer.position(info.offset);
                    outputStream.write(outByteBuffer, 0, perpcmsize);
                    mediaCodec.releaseOutputBuffer(outPutBufferIndex, false);
                    outPutBufferIndex = mediaCodec.dequeueOutputBuffer(info, 0);
                    outByteBuffer = null;
                }
            } catch (Exception e) {
                Log.d(TAG, " encodePcmTAAc error: " + e.getMessage());
                e.printStackTrace();
            }
        }
    }

    private int getADTSsamplerate(int samplerate) {
        int rate = 4;
        switch (samplerate) {
            case 96000:
                rate = 0;
                break;
            case 88200:
                rate = 1;
                break;
            case 64000:
                rate = 2;
                break;
            case 48000:
                rate = 3;
                break;
            case 44100:
                rate = 4;
                break;
            case 32000:
                rate = 5;
                break;
            case 24000:
                rate = 6;
                break;
            case 22050:
                rate = 7;
                break;
            case 16000:
                rate = 8;
                break;
            case 12000:
                rate = 9;
                break;
            case 11025:
                rate = 10;
                break;
            case 8000:
                rate = 11;
                break;
            case 7350:
                rate = 12;
                break;
        }
        return rate;
    }

    /**
     * ADTS 介绍
     *
     * @param packet
     * @param packetLen
     * @link{https://blog.csdn.net/jay100500/article/details/52955232}
     */
    private void addADtsHeader(byte[] packet, int packetLen, int samplerate) {
        int profile = TXConstant.MEDIA_CODEC_PROFILE; // AAC LC
        int freqIdx = samplerate; // 44.1KHz
        int chanCfg = TXConstant.CHANNEL_COUNT; // CPE

        // fill in ADTS data
        packet[0] = (byte) 0xFF;
        packet[1] = (byte) 0xF9;
        packet[2] = (byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        packet[3] = (byte) (((chanCfg & 3) << 6) + (packetLen >> 11));
        packet[4] = (byte) ((packetLen & 0x7FF) >> 3);
        packet[5] = (byte) (((packetLen & 7) << 5) + 0x1F);
        packet[6] = (byte) 0xFC;
    }
}
