package com.taxiao.ffmpeg.utils;

import android.media.MediaCodecInfo;

/**
 * Created by hanqq on 2021/5/21
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class TXConstant {

    // 声道切换 0右声道 1左声道 2立体声
    public static int CHANNEL_RIGHT = 0;
    public static int CHANNEL_LEFT = 1;
    public static int CHANNEL_STEREO = 2;
    public static int CHANNEL_COUNT = CHANNEL_STEREO;
    public static int MEDIA_CODEC_PROFILE = MediaCodecInfo.CodecProfileLevel.AACObjectLC;
}
