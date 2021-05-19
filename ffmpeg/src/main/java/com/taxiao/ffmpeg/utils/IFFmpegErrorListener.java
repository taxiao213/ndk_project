package com.taxiao.ffmpeg.utils;

/**
 * 错误的回调
 * Created by hanqq on 2021/5/18
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public interface IFFmpegErrorListener {
    void error(int code, String errorMsg);
}
