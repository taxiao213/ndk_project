package com.taxiao.ffmpeg;

import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;

import com.taxiao.ffmpeg.utils.IFFmpegCompleteListener;
import com.taxiao.ffmpeg.utils.IFFmpegCutAudioListener;
import com.taxiao.ffmpeg.utils.IFFmpegDecodeVideoListener;
import com.taxiao.ffmpeg.utils.IFFmpegErrorListener;
import com.taxiao.ffmpeg.utils.IFFmpegGLSurfaceListener;
import com.taxiao.ffmpeg.utils.IFFmpegParparedListener;
import com.taxiao.ffmpeg.utils.IFFmpegRecordTimeListener;
import com.taxiao.ffmpeg.utils.IFFmpegTimeListener;
import com.taxiao.ffmpeg.utils.IFFmpegValumeDBListener;
import com.taxiao.ffmpeg.utils.TXMediaCodecVideoUtil;
import com.taxiao.ffmpeg.utils.TXMediacodecUtil;
import com.taxiao.ffmpeg.utils.TXVideoSupportUitl;
import com.taxiao.ffmpeg.utils.TimeInfoModel;

import java.io.File;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * ffmpeg 播放器
 * Created by hanqq on 2021/3/30
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class JniSdkImpl {
    private String TAG = JniSdkImpl.this.getClass().getSimpleName();

    static {
        if (BuildConfig.FFMPEG_BUILD_VERSION == 1) {
            System.loadLibrary("avcodec-57");
            System.loadLibrary("avdevice-57");
            System.loadLibrary("avfilter-6");
            System.loadLibrary("avformat-57");
            System.loadLibrary("avutil-55");
            System.loadLibrary("postproc-54");
            System.loadLibrary("swresample-2");
            System.loadLibrary("swscale-4");
        } else if (BuildConfig.FFMPEG_BUILD_VERSION == 2) {
            System.loadLibrary("avcodec");
            System.loadLibrary("avdevice");
            System.loadLibrary("avfilter");
            System.loadLibrary("avformat");
            System.loadLibrary("avutil");
            System.loadLibrary("postproc");
            System.loadLibrary("swresample");
            System.loadLibrary("swscale");
        }
        System.loadLibrary("native-lib");
    }

    private String filePath;
    private ExecutorService executorService;
    private IFFmpegParparedListener iffmpegparparedlistener;
    private IFFmpegTimeListener iffmpegtimelistener;
    private IFFmpegErrorListener iffmpegerrorlistener;
    private IFFmpegCompleteListener iffmpegcompletelistener;
    private IFFmpegValumeDBListener iffmpegValumeDBListener;
    private IFFmpegRecordTimeListener iffmpegRecordTimeListener;
    private IFFmpegCutAudioListener iffmpegCutAudioListener;
    private IFFmpegDecodeVideoListener iffmpegDecodeVideoListener;
    private IFFmpegGLSurfaceListener iffmpegGLSurfaceListener;
    private MyCallBack myCallBack;
    private static TimeInfoModel timeInfoModel;
    private static int volumePercent = 100;


    public JniSdkImpl() {
        executorService = Executors.newSingleThreadExecutor();
    }

    public void setOnCallBack(MyCallBack myCallBack) {
        this.myCallBack = myCallBack;
    }

    public void setIFFmpegParparedListener(IFFmpegParparedListener fmpegParparedListener) {
        this.iffmpegparparedlistener = fmpegParparedListener;
    }

    public void setIFFmpegTimeListener(IFFmpegTimeListener fFmpegTimeListener) {
        this.iffmpegtimelistener = fFmpegTimeListener;
    }

    public void setIFFmpegErrorListener(IFFmpegErrorListener fFmpegErrorListener) {
        this.iffmpegerrorlistener = fFmpegErrorListener;
    }

    public void setIFFmpegCompleteListener(IFFmpegCompleteListener fFmpegCompleteListener) {
        this.iffmpegcompletelistener = fFmpegCompleteListener;
    }

    public void setIFFmpegValumeDBListener(IFFmpegValumeDBListener fFmpegValumeDBListener) {
        this.iffmpegValumeDBListener = fFmpegValumeDBListener;
    }

    public void setIFFmpegRecordTimeListener(IFFmpegRecordTimeListener fFmpegRecordTimeListener) {
        this.iffmpegRecordTimeListener = fFmpegRecordTimeListener;
    }

    public void setIFFmpegCutAudioListener(IFFmpegCutAudioListener fFmpegCutAudioListener) {
        this.iffmpegCutAudioListener = fFmpegCutAudioListener;
    }

    public void setIFFmpegDecodeVideoListener(IFFmpegDecodeVideoListener fFmpegDecodeVideoListener) {
        this.iffmpegDecodeVideoListener = fFmpegDecodeVideoListener;
    }

    public void setFFmpegGLSurfaceListener(IFFmpegGLSurfaceListener fFmpegGLSurfaceListener) {
        this.iffmpegGLSurfaceListener = fFmpegGLSurfaceListener;
    }

    public void setSource(String filePath) {
        this.filePath = filePath;
    }

    public void parpared() {
        if (!TextUtils.isEmpty(filePath)) {
            executorService.execute(new Runnable() {
                @Override
                public void run() {
                    n_parpared(filePath);
                }
            });
        }
    }

    public void setVolume(int percent) {
        if (percent >= 0 && percent <= 100) {
            volumePercent = percent;
            n_volume(percent);
        }
    }

    public void setMute(int channel) {
        n_mute(channel);
    }

    public int getVolumePercent() {
        return volumePercent;
    }

    public void setPitch(float pitch) {
        n_pitch(pitch);
    }

    public void setSpeed(float speed) {
        n_speed(speed);
    }

    // 开始录音
    public void startRecord(File file) {
        int sampleRate = n_getSampleRate();
        Log.d(TAG, "resumeRecord sampleRate : " + sampleRate);
        if (sampleRate > 0) {
            TXMediacodecUtil.getInstance().createAudioCodec(sampleRate, file);
            n_startRecord();
        }
    }

    // 暂停 恢复录音 true 恢复 false 暂停
    public void resumeRecord(boolean isStart) {
        n_resumeRecord(isStart);
    }

    // 停止录音
    public void stopRecord() {
        TXMediacodecUtil.getInstance().release();
        n_stopRecord();
    }

    public void cutAudio(int startTime, int endTime, boolean isShowPcm, String cutPath) {
        n_cutAudio(startTime, endTime, isShowPcm, cutPath);
    }

    public void stop() {
        TXMediaCodecVideoUtil.getInstance().release();
        TXMediacodecUtil.getInstance().release();
        n_stop();
    }

    // -------------------------   c++ 回调函数 --------------------------------------

    /**
     * c++ 时间回调
     *
     * @param currentTime
     * @param totalTime
     */
    public void callTimeInfo(int currentTime, int totalTime) {
        if (iffmpegtimelistener != null) {
            if (timeInfoModel == null) {
                timeInfoModel = new TimeInfoModel();
            }
            timeInfoModel.setCurrentTime(currentTime);
            timeInfoModel.setTotalTime(totalTime);
            iffmpegtimelistener.onTimeInfo(timeInfoModel);
        }
    }

    /**
     * c++ 准备就绪回调
     */
    public void callParpared() {
        if (iffmpegparparedlistener != null) {
            iffmpegparparedlistener.parpared();
        }
    }

    /**
     * c++
     */
    public void callOnLoad(boolean isLoad) {
        if (iffmpegparparedlistener != null) {
            iffmpegparparedlistener.onLoad(isLoad);
        }
    }

    /**
     * 开始
     */
    public void callOnResume() {
        n_resume();
        if (iffmpegparparedlistener != null) {
            iffmpegparparedlistener.onResume();
        }
    }

    /**
     * 暂停
     */
    public void callOnPause() {
        n_pause();
        if (iffmpegparparedlistener != null) {
            iffmpegparparedlistener.onPause();
        }
    }

    /**
     * 错误的回调
     *
     * @param code
     * @param name
     */
    public void callOnError(int code, String name) {
        Log.d("sdk callOnError ", "code :" + code + " name: " + name);
        if (iffmpegerrorlistener != null) {
            iffmpegerrorlistener.error(code, name);
        }
        stop();
    }

    /**
     * 播放完成的回调
     */
    public void callOnComplete() {
        stop();
        if (iffmpegcompletelistener != null) {
            iffmpegcompletelistener.complete();
        }
    }

    /**
     * 音量分贝值回调
     */
    public void callOnValumeDB(int db) {
        if (iffmpegValumeDBListener != null) {
            iffmpegValumeDBListener.onDbValue(db);
        }
    }

    public void callOnPcmTAAc(int size, byte[] buffer) {
        TXMediacodecUtil.getInstance().encodePcmTAAc(size, buffer);
    }

    public void callOnRecordTime(float time) {
        if (iffmpegRecordTimeListener != null) {
            iffmpegRecordTimeListener.onRecordTime(time);
        }
    }

    public void callOnCutAudio(int sampleRate, byte[] buffer) {
        if (iffmpegCutAudioListener != null) {
            iffmpegCutAudioListener.cutAudio(sampleRate, buffer);
        }
    }

    public void callOnRenderYUV(int width, int height, byte[] y, byte[] u, byte[] v) {
        Log.d(TAG, "获取到视频的yuv数据, width: " + width + " height: " + height);
        if (iffmpegDecodeVideoListener != null) {
            iffmpegDecodeVideoListener.renderType(true);
            iffmpegDecodeVideoListener.onRenderYUV(width, height, y, u, v);
        }
    }

    public boolean callOnIsSupportMediaCodec(String ffcodecname) {
        Log.d(TAG, "callOnIsSupportMediaCodec  ffcodecname: " + ffcodecname);
        return TXVideoSupportUitl.isSupportCodec(ffcodecname);
    }

    // 初始化 video mediacodec
    public void callOnInitMediaCodecVideo(String mime, int width, int height, byte[] csd_01, byte[] csd_02) {
        Log.d(TAG, "callOnInitMediaCodecVideo ");
        if (iffmpegGLSurfaceListener != null) {
            Surface surface = iffmpegGLSurfaceListener.getSurface();
            if (surface != null) {
                Log.d(TAG, "callOnInitMediaCodecVideo surface != null ");
                if (iffmpegDecodeVideoListener != null) {
                    iffmpegDecodeVideoListener.renderType(false);
                }
                TXMediaCodecVideoUtil.getInstance().createVideoCodec(surface, mime, width, height, csd_01, csd_02);
            }
        }
    }

    // 渲染数据
    public void callOnDecodeAvPacket(int size, byte[] buffer) {
        TXMediaCodecVideoUtil.getInstance().decodeAvPacket(size, buffer);
    }

    public void setSurface(Surface surface) {

    }

    public interface MyCallBack {
        void error(int code, String name);

        void success();
    }

    public void callError(int code, String name) {
        if (myCallBack != null)
            myCallBack.error(code, name);
    }

    // 播放 pcm 格式音乐
    public void playPcm(String path) {
        testPlay(path);
    }

    public native void startThread();

    public native void mutexThread();

    public native void javaMain2C();

    public native void javaThread2C();

    public native void testFFmpeg();

    public native void n_parpared(String path);

    public native void n_start();

    public native void n_resume();

    public native void n_pause();

    public native void n_stop();

    // seek
    public native void n_seek(int seconds);

    // 音量
    public native void n_volume(int percent);

    public native void testPlay(String path);

    // 设置声道
    public native void n_mute(int channel);

    // 设置变调
    private native void n_pitch(float pitch);

    // 设置变速
    private native void n_speed(float speed);

    // 开始录音
    private native void n_startRecord();

    // 是否继续录音  true 继续  false 暂停录音
    private native void n_resumeRecord(boolean resumeRecord);

    // 停止录音
    private native void n_stopRecord();

    // 返回 sampleRate
    private native int n_getSampleRate();

    private native void n_cutAudio(int startTime, int endTime, boolean isShowPcm, String cutPath);

    public native void test();

}
