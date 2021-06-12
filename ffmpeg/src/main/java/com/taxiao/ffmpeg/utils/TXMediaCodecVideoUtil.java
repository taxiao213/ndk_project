package com.taxiao.ffmpeg.utils;

import android.media.FaceDetector;
import android.media.MediaCodec;
import android.media.MediaFormat;
import android.os.Handler;
import android.os.HandlerThread;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceView;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * MediaCodec 解码 h264
 * Created by hanqq on 2021/6/5
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class TXMediaCodecVideoUtil {
    private static final String TAG = TXMediaCodecVideoUtil.class.getSimpleName();
    private static volatile TXMediaCodecVideoUtil mMediaCodecUtils;
    private HandlerThread handlerThread;
    private TXMediaCodecVideoUtil.VideoRunnable videoRunnable;
    private String mime;
    private int width;
    private int height;
    private byte[] csd_0;
    private byte[] csd_1;
    private MediaCodec mediaCodec;
    private MediaFormat videoFormat;
    private MediaCodec.BufferInfo info = null;
    private Surface surface = null;

    private TXMediaCodecVideoUtil() {
    }

    public static TXMediaCodecVideoUtil getInstance() {
        if (mMediaCodecUtils == null) {
            synchronized (TXMediaCodecVideoUtil.class) {
                if (mMediaCodecUtils == null) {
                    mMediaCodecUtils = new TXMediaCodecVideoUtil();
                }
            }
        }
        return mMediaCodecUtils;
    }

    /**
     * 初始化 video codec
     *
     * @param codecName
     * @param width
     * @param height
     * @param csd_0     SPS（序列参数集*）
     * @param csd_1     PPS（图片参数集*）
     */
    public void createVideoCodec(Surface surface, String codecName, int width, int height, byte[] csd_0, byte[] csd_1) {
        this.surface = surface;
        this.mime = TXVideoSupportUitl.findVideoCodecName(codecName);
        Log.d(TAG, "createVideoCodec : codecName " + codecName + " mime: " + mime);
        this.width = width;
        this.height = height;
        this.csd_0 = csd_0;
        this.csd_1 = csd_1;
        handlerThread = new HandlerThread("video_codec");
        handlerThread.start();
        Handler handler = new Handler(handlerThread.getLooper());
        videoRunnable = new TXMediaCodecVideoUtil.VideoRunnable();
        handler.post(videoRunnable);
    }

    public class VideoRunnable implements Runnable {
        @Override
        public void run() {
            if (surface != null) {
                try {
                    Log.d(TAG, "VideoRunnable mime : " + mime);
                    videoFormat = MediaFormat.createVideoFormat(mime, width, height);
                    videoFormat.setInteger(MediaFormat.KEY_WIDTH, width);
                    videoFormat.setInteger(MediaFormat.KEY_HEIGHT, height);
                    videoFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, width * height);
                    if (TextUtils.equals(mime, MediaFormat.MIMETYPE_VIDEO_AVC)) {
                        videoFormat.setByteBuffer("csd-0", ByteBuffer.wrap(csd_0));
                        videoFormat.setByteBuffer("csd-1", ByteBuffer.wrap(csd_1));
                    }
                    mediaCodec = MediaCodec.createDecoderByType(mime);
                    mediaCodec.configure(videoFormat, surface, null, 0);
                    info = new MediaCodec.BufferInfo();
                    mediaCodec.start();
                } catch (Exception e) {
                    Log.d(TAG, "mediaCodec = null " + e.getMessage());
                    e.printStackTrace();
                }
            }
        }
    }

    /**
     * 解码 AVPacket
     *
     * @param size
     * @param buffer
     */
    public void decodeAvPacket(int size, byte[] buffer) {
        Log.d(TAG, "decodeAvPacket size: " + size + " buffer: " + buffer.length + " mediaCodec: " + mediaCodec);
        if (surface != null && mediaCodec != null && buffer.length > 0) {
            try {
                Log.d(TAG, "decodeAvPacket");
                int inputBufferIndex = mediaCodec.dequeueInputBuffer(10);
                if (inputBufferIndex >= 0) {
                    ByteBuffer inputBuffer = mediaCodec.getInputBuffers()[inputBufferIndex];
                    inputBuffer.clear();
                    inputBuffer.put(buffer);
                    mediaCodec.queueInputBuffer(inputBufferIndex, 0, size, 0, 0);
                }
                if (info != null) {
                    int outputBufferIndex = mediaCodec.dequeueOutputBuffer(info, 10);
                    while (outputBufferIndex >= 0) {
                        mediaCodec.releaseOutputBuffer(outputBufferIndex, true);
                        outputBufferIndex = mediaCodec.dequeueOutputBuffer(info, 10);
                    }
                }
            } catch (Exception e) {
                Log.d(TAG, "decodeAvPacket Exception " + e.getMessage());
                e.printStackTrace();
            }
        }
    }

    public void release() {
        Log.d(TAG, " mediacode release");
        try {
            if (handlerThread != null) {
                handlerThread.getLooper().quit();
                handlerThread.quitSafely();
                handlerThread = null;
            }
            if (videoRunnable != null) {
                videoRunnable = null;
            }
            if (videoFormat != null) {
                videoFormat = null;
            }
            if (mediaCodec != null) {
                mediaCodec.stop();
                mediaCodec.release();
                mediaCodec = null;
            }
            if (info != null) {
                info = null;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
