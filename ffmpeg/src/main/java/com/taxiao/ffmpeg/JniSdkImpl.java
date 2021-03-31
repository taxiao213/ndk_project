package com.taxiao.ffmpeg;

/**
 * Created by hanqq on 2021/3/30
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class JniSdkImpl {
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
    }

    public JniSdkImpl() {
    }

    private MyCallBack myCallBack;

    public void setOnCallBack(MyCallBack myCallBack) {
        this.myCallBack = myCallBack;
    }

    public interface MyCallBack {
        void error(int code, String name);

        void success();
    }

    public void callError(int code, String name) {
        if (myCallBack != null)
            myCallBack.error(code, name);
    }

    public native void startThread();

    public native void mutexThread();

    public native void javaMain2C();

    public native void javaThread2C();

    public native void testFFmpeg();
}
