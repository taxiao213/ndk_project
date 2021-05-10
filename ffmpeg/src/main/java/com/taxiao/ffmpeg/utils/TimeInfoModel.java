package com.taxiao.ffmpeg.utils;

/**
 * Created by hanqq on 2021/5/10
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class TimeInfoModel {
    int currentTime;
    int totalTime;

    public TimeInfoModel() {
    }

    public TimeInfoModel(int currentTime, int totalTime) {
        this.currentTime = currentTime;
        this.totalTime = totalTime;
    }

    public int getCurrentTime() {
        return currentTime;
    }

    public void setCurrentTime(int currentTime) {
        this.currentTime = currentTime;
    }

    public int getTotalTime() {
        return totalTime;
    }

    public void setTotalTime(int totalTime) {
        this.totalTime = totalTime;
    }
}
