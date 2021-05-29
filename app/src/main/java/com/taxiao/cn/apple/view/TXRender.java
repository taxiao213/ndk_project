package com.taxiao.cn.apple.view;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by hanqq on 2021/5/29
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class TXRender implements GLSurfaceView.Renderer {

    // 顶点坐标
    private float[] vertex = {
            -1.0f, 0f,
            0f, 1.0f,
            1.0f, 0f
    };

    // floatbuffer
    private FloatBuffer vertexBuffer;

    public TXRender() {
        // 4：float 占用的字节数
        vertexBuffer = ByteBuffer.allocateDirect(vertex.length * 4)
                // 对齐方式 有两种 BIG_ENDIAN  LITTLE_ENDIAN  nativeOrder 和本地机器一样
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                // 映射顶点坐标
                .put(vertex);
        vertexBuffer.position(0);


    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {

    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        // 清屏
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        // 红色清屏
        GLES20.glClearColor(1.0f, 1.0f, 0, 1);
    }
}
