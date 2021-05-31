package com.taxiao.ffmpeg.utils;

/**
 * YUV 数据渲染
 * Created by hanqq on 2021/4/14
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public interface IFFmpegDecodeVideoListener {

    void onRenderYUV(int width, int height, byte[] y, byte[] u, byte[] v);
}
