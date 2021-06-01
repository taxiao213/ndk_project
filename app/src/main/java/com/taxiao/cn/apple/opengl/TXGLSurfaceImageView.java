package com.taxiao.cn.apple.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

/**
 * 渲染图片
 * ffmpeg decode
 * <p>
 * Created by hanqq on 2021/5/31
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class TXGLSurfaceImageView extends GLSurfaceView {

    public TXGLSurfaceImageView(Context context) {
        this(context, null);
    }

    public TXGLSurfaceImageView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        setRenderer(new TXImageRender(context));
    }
}
