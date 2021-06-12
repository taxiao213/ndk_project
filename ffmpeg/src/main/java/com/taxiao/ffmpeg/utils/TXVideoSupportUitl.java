package com.taxiao.ffmpeg.utils;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.text.TextUtils;
import android.util.Log;
import android.view.TextureView;

import java.util.HashMap;
import java.util.Map;

/**
 * 是否支持硬编码
 * Created by hanqq on 2021/6/3
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class TXVideoSupportUitl {
    private static final String TAG = TXVideoSupportUitl.class.getSimpleName();

    private static Map<String, String> codecMap = new HashMap<>();

    static {
        codecMap.put("h264", "video/avc");
        codecMap.put("h265", "video/hevc");
    }

    public static String findVideoCodecName(String ffcodename) {
        if (codecMap.containsKey(ffcodename)) {
            return codecMap.get(ffcodename);
        }
        return "";
    }

    public static boolean isSupportCodec(String ffcodecname) {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
            MediaCodecList mediaCodecList = new MediaCodecList(MediaCodecList.ALL_CODECS);
            MediaCodecInfo[] codecInfos = mediaCodecList.getCodecInfos();
            for (MediaCodecInfo mediaCodecInfo : codecInfos) {
                if (mediaCodecInfo != null) {
                    String name = mediaCodecInfo.getName();
                    String[] supportedTypes = mediaCodecInfo.getSupportedTypes();
                    for (String type : supportedTypes) {
                        Log.d(TAG, " isSupportCodec name: " + name + " ,type: " + type);
                        if (TextUtils.equals(findVideoCodecName(ffcodecname), type)) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }
}
