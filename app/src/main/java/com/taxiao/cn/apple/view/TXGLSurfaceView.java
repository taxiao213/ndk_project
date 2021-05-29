package com.taxiao.cn.apple.view;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

/**
 * Created by hanqq on 2021/5/29
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class TXGLSurfaceView extends GLSurfaceView {
    public TXGLSurfaceView(Context context) {
        this(context, null);
    }

    public TXGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        setRenderer(new TXRender());
    }
}
