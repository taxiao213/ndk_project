package com.taxiao.ffmpeg;

import android.text.TextUtils;
import android.util.Log;

import com.taxiao.ffmpeg.utils.IFFmpegCompleteListener;
import com.taxiao.ffmpeg.utils.IFFmpegErrorListener;
import com.taxiao.ffmpeg.utils.IFFmpegParparedListener;
import com.taxiao.ffmpeg.utils.IFFmpegTimeListener;
import com.taxiao.ffmpeg.utils.TimeInfoModel;

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
        n_stop();
    }

    /**
     * 播放完成的回调
     */
    public void callOnComplete() {
        if (iffmpegcompletelistener != null) {
            iffmpegcompletelistener.complete();
        }
        n_stop();
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

}
