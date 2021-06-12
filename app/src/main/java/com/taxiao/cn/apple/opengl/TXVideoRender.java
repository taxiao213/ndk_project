package com.taxiao.cn.apple.opengl;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.view.Surface;


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
public class TXVideoRender implements GLSurfaceView.Renderer, SurfaceTexture.OnFrameAvailableListener {
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
    public static final int RENDER_YUV = 1;
    public static final int RENDER_MEDIACODEC = 2;
    private int renderType = RENDER_YUV;

    private int mediacodecProgram;
    private int mediacodec_av_position;
    private int mediacodec_af_position;
    private int[] mediacodec_textureId;
    private Surface surface;
    private SurfaceTexture surfaceTexture;
    private int mediacodec_sTexture;
    private GLOnFrameAvailable glOnFrameAvailable;

    public TXVideoRender(Context context) {
        this.mContext = context;
        // 1.创建顶点 和 纹理 buffer
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
        initRenderMediaCodec();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glClearColor(0f, 0f, 0f, 1f);
        if (renderType == RENDER_YUV) {
            renderYUV();
        } else if (renderType == RENDER_MEDIACODEC) {
            renderMediaCodec();
        }
        // 12.绘制四边形
        // renderYUV 判断条件后会导致闪屏
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
    }

    private void initRenderYUV() {
        LogUtils.d(TAG, "initRenderYUV ");
        // 2.加载 shader
        String vertexShader = ShaderUtils.readRawTxt(mContext, R.raw.vertex_video_shader);
        String fragmentShader = ShaderUtils.readRawTxt(mContext, R.raw.fragment_video_shader);
        // 3.创建渲染程序
        program = ShaderUtils.createProgram(vertexShader, fragmentShader);
        LogUtils.d(TAG, "program " + program);
        if (program > 0) {
            // 4.得到着色器中的属性
            av_position = GLES20.glGetAttribLocation(program, "av_Position");
            af_position = GLES20.glGetAttribLocation(program, "af_Position");

            sampler_y = GLES20.glGetUniformLocation(program, "sampler_y");
            sampler_u = GLES20.glGetUniformLocation(program, "sampler_u");
            sampler_v = GLES20.glGetUniformLocation(program, "sampler_v");
            textureId_yuv = new int[3];
            // 5.创建纹理
            GLES20.glGenTextures(3, textureId_yuv, 0);
            for (int i = 0; i < 3; i++) {
                // 6.绑定纹理
                GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[i]);
                // 7.设置环绕和过滤方式 环绕（超出纹理坐标范围）：（s==x t==y GL_REPEAT 重复）
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
                // 8.过滤（纹理像素映射到坐标点）：（缩小、放大：GL_LINEAR线性）
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            }
        }
    }

    private void renderYUV() {
        LogUtils.d(TAG, "renderYUV ");
        if (width_yuv > 0 && height_yuv > 0 && y != null && u != null && v != null /*&& program > 0*/) {
            // 9.使用渲染器
            GLES20.glUseProgram(program);
            // 10.使顶点坐标和纹理坐标属性数组有效
            GLES20.glEnableVertexAttribArray(av_position);
            GLES20.glVertexAttribPointer(av_position, 2, GLES20.GL_FLOAT, false, 8, vetexBuffer);

            GLES20.glEnableVertexAttribArray(af_position);
            GLES20.glVertexAttribPointer(af_position, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);

            // 11.纹理赋值
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
        }
    }

    public void setYUVRenderData(int width, int height, byte[] y, byte[] u, byte[] v) {
        LogUtils.d(TAG, "setYUVRenderData ");
        this.width_yuv = width;
        this.height_yuv = height;
        this.y = ByteBuffer.wrap(y);
        this.u = ByteBuffer.wrap(u);
        this.v = ByteBuffer.wrap(v);
    }

    private void initRenderMediaCodec() {
        LogUtils.d(TAG, "initRenderMediaCodec ");
        // 加载shader
        String vertexShader = ShaderUtils.readRawTxt(mContext, R.raw.vertex_video_mediacodec_shader);
        String fragmentShader = ShaderUtils.readRawTxt(mContext, R.raw.fragment_video_mediacodec_shader);
        mediacodecProgram = ShaderUtils.createProgram(vertexShader, fragmentShader);
        if (mediacodecProgram > 0) {
            // 4.得到着色器中的属性
            mediacodec_av_position = GLES20.glGetAttribLocation(mediacodecProgram, "av_Position");
            mediacodec_af_position = GLES20.glGetAttribLocation(mediacodecProgram, "af_Position");
            mediacodec_sTexture = GLES20.glGetUniformLocation(mediacodecProgram, "sTexture");

            mediacodec_textureId = new int[1];
            // 5.创建纹理
            GLES20.glGenTextures(1, mediacodec_textureId, 0);

            // 6.绑定纹理

            // 7.设置环绕和过滤方式 环绕（超出纹理坐标范围）：（s==x t==y GL_REPEAT 重复）
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
            // 8.过滤（纹理像素映射到坐标点）：（缩小、放大：GL_LINEAR线性）
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            surfaceTexture = new SurfaceTexture(mediacodec_textureId[0]);
            surface = new Surface(surfaceTexture);
            surfaceTexture.setOnFrameAvailableListener(this);
        }
    }

    /**
     * mediacodec 渲染
     */
    private void renderMediaCodec() {
        LogUtils.d(TAG, "renderMediaCodec ");
        if (surfaceTexture != null) {
            surfaceTexture.updateTexImage();
            GLES20.glUseProgram(mediacodecProgram);
            GLES20.glEnableVertexAttribArray(av_position);
            GLES20.glVertexAttribPointer(av_position, 2, GLES20.GL_FLOAT, false, 8, vetexBuffer);

            GLES20.glEnableVertexAttribArray(af_position);
            GLES20.glVertexAttribPointer(af_position, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mediacodec_textureId[0]);
            GLES20.glUniform1i(mediacodec_sTexture, 0);
        }
    }

    public Surface getSurface() {
        return surface;
    }

    public void setRenderType(int renderType) {
        this.renderType = renderType;
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        if (glOnFrameAvailable != null) {
            glOnFrameAvailable.onAvailable();
        }
    }

    public void setGlOnFrameAvailable(GLOnFrameAvailable glOnFrameAvailable) {
        this.glOnFrameAvailable = glOnFrameAvailable;
    }

    public interface GLOnFrameAvailable {
        void onAvailable();
    }

    public interface GLOnSurfaceCreateListener {
        void onSurfaceCreate(Surface surface);
    }
}
