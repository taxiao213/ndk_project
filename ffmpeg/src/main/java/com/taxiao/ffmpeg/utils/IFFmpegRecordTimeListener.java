package com.taxiao.ffmpeg.utils;

/**
 * 录音时间回调
 * Created by hanqq on 2021/4/14
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public interface IFFmpegRecordTimeListener {
    void onRecordTime(float time);
}
