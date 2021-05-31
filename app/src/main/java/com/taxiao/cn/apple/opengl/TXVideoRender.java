package com.taxiao.cn.apple.opengl;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;


import com.taxiao.cn.apple.R;
import com.taxiao.cn.apple.util.LogUtils;
import com.taxiao.cn.apple.util.ShaderUtils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by hanqq on 2021/5/31
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class TXVideoRender implements GLSurfaceView.Renderer {
    private String TAG = TXVideoRender.this.getClass().getSimpleName();

    // 顶点坐标
    private final float[] vertexData = {
            -1f, -1f,
            1f, -1f,
            -1f, 1f,
            1f, 1f
    };

    // 纹理坐标
    private final float[] textureData = {
            0f, 1f,
            1f, 1f,
            0f, 0f,
            1f, 0f
    };

    private Context mContext;
    private FloatBuffer vetexBuffer;
    private FloatBuffer textureBuffer;
    private int program;
    private int av_position;
    private int af_position;

    private int[] textureId_yuv;
    private int sampler_y;
    private int sampler_u;
    private int sampler_v;

    private int width_yuv;
    private int height_yuv;
    private ByteBuffer y;
    private ByteBuffer u;
    private ByteBuffer v;

    public TXVideoRender(Context context) {
        this.mContext = context;
        vetexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(vertexData);
        vetexBuffer.position(0);

        textureBuffer = ByteBuffer.allocateDirect(textureData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(textureData);
        textureBuffer.position(0);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        initRenderYUV();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glClearColor(0f, 0f, 0f, 1f);
        renderYUV();
    }

    private void initRenderYUV() {
        String vertexShader = ShaderUtils.readRawTxt(mContext, R.raw.vertex_shader1);
        String fragmentShader = ShaderUtils.readRawTxt(mContext, R.raw.fragment_shader1);
        program = ShaderUtils.createProgram(vertexShader, fragmentShader);
        LogUtils.d(TAG, "program " + program);
        if (program > 0) {
            // 9.得到着色器中的属性
            av_position = GLES20.glGetAttribLocation(program, "av_Position");
            af_position = GLES20.glGetAttribLocation(program, "af_Position");

            sampler_y = GLES20.glGetUniformLocation(program, "sampler_y");
            sampler_u = GLES20.glGetUniformLocation(program, "sampler_u");
            sampler_v = GLES20.glGetUniformLocation(program, "sampler_v");
            textureId_yuv = new int[3];
            // 创建纹理
            GLES20.glGenTextures(3, textureId_yuv, 0);
            for (int i = 0; i < 3; i++) {
                // 绑定纹理
                GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[i]);
                // 设置环绕和过滤方式 环绕（超出纹理坐标范围）：（s==x t==y GL_REPEAT 重复）
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
                // 过滤（纹理像素映射到坐标点）：（缩小、放大：GL_LINEAR线性）
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            }
        }
    }

    private void renderYUV() {
        if (width_yuv > 0 && height_yuv > 0 && y != null && u != null && v != null /*&& program > 0*/) {
            GLES20.glUseProgram(program);
            // 11.使顶点属性数组有效
            GLES20.glEnableVertexAttribArray(av_position);
            GLES20.glVertexAttribPointer(av_position, 2, GLES20.GL_FLOAT, false, 8, vetexBuffer);

            GLES20.glEnableVertexAttribArray(af_position);
            GLES20.glVertexAttribPointer(af_position, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);

            // 纹理赋值
            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[0]);
            // level 级别 border 边框
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv, height_yuv, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, y);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[1]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv / 2, height_yuv / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, u);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[2]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv / 2, height_yuv / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, v);

            GLES20.glUniform1i(sampler_y, 0);
            GLES20.glUniform1i(sampler_u, 1);
            GLES20.glUniform1i(sampler_v, 2);

            y.clear();
            u.clear();
            v.clear();
            y = null;
            u = null;
            v = null;

            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    public void setYUVRenderData(int width, int height, byte[] y, byte[] u, byte[] v) {
        this.width_yuv = width;
        this.height_yuv = height;
        this.y = ByteBuffer.wrap(y);
        this.u = ByteBuffer.wrap(u);
        this.v = ByteBuffer.wrap(v);
    }

}
