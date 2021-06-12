package com.taxiao.cn.apple.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.Surface;

/**
 * 渲染视频
 * ffmpeg decode
 * <p>
 * Created by hanqq on 2021/5/31
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class TXGLSurfaceVideoView extends GLSurfaceView {

    private TXVideoRender renderer;

    public TXGLSurfaceVideoView(Context context) {
        this(context, null);
    }

    public TXGLSurfaceVideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        renderer = new TXVideoRender(context);
        setRenderer(renderer);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        renderer.setGlOnFrameAvailable(new TXVideoRender.GLOnFrameAvailable() {
            @Override
            public void onAvailable() {
                requestRender();
            }
        });
    }

    public void setYUVData(int width, int height, byte[] y, byte[] u, byte[] v) {
        if (renderer != null) {
            renderer.setYUVRenderData(width, height, y, u, v);
            requestRender();
        }
    }


    public Surface getSurface() {
        if (renderer != null) {
            return renderer.getSurface();
        }
        return null;
    }

    public void setRenderType(boolean renderYUV) {
        if (renderer != null) {
            renderer.setRenderType(renderYUV ? TXVideoRender.RENDER_YUV : TXVideoRender.RENDER_MEDIACODEC);
        }
    }

}
