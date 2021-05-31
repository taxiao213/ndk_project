package com.taxiao.cn.apple.opengl;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import com.taxiao.cn.apple.R;
import com.taxiao.cn.apple.util.ShaderUtils;

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

    private Context mContext;
    // 顶点坐标 绘制三角形
    private float[] vertex = {
            -1.0f, 0f,
            0f, 1.0f,
            1.0f, 0f
    };

    // 顶点坐标 绘制四角形 6 个点
    private float[] vertex2 = {
            -1.0f, 0f,
            0f, 1.0f,
            1.0f, 0f,
            -1.0f, 0f,
            1.0f, 0f,
            0f, -1f
    };

    // 顶点坐标 绘制四角形 4 个点
    private float[] vertex3 = {
            -1.0f, 0f,
            0f, -1f,
            0f, 1f,
            1f, 0f
    };

    // floatbuffer 申请内存空间
    private FloatBuffer vertexBuffer;
    private int program;
    private int av_position;
    private int af_potion;
    private int quadrilateralType = 2;// 0 三角形(3点绘制)   1 四边形(6点绘制)  2 四边形(4点绘制)

    public TXRender(Context context) {
        this.mContext = context;
        // floatbuffer 申请内存空间
        // 4：float 占用的字节数
        if (quadrilateralType == 0) {
            // 绘制三角形 Buffer
            vertexBuffer = ByteBuffer.allocateDirect(vertex.length * 4)
                    // 对齐方式 有两种 BIG_ENDIAN  LITTLE_ENDIAN  nativeOrder 和本地机器一样
                    .order(ByteOrder.nativeOrder())
                    .asFloatBuffer()
                    // 映射顶点坐标
                    .put(vertex);
        } else if (quadrilateralType == 1) {
            // 绘制四边形 Buffer
            vertexBuffer = ByteBuffer.allocateDirect(vertex2.length * 4)
                    .order(ByteOrder.nativeOrder())
                    .asFloatBuffer()
                    .put(vertex2);
        } else if (quadrilateralType == 2) {
            // 绘制四边形 Buffer
            vertexBuffer = ByteBuffer.allocateDirect(vertex3.length * 4)
                    // 对齐方式 有两种 BIG_ENDIAN  LITTLE_ENDIAN  nativeOrder 和本地机器一样
                    .order(ByteOrder.nativeOrder())
                    .asFloatBuffer()
                    // 映射顶点坐标
                    .put(vertex3);
        }
        vertexBuffer.position(0);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        String vertexShader = ShaderUtils.readRawTxt(mContext, R.raw.vertex_shader);
        String fragmentShader = ShaderUtils.readRawTxt(mContext, R.raw.fragment_shader);
        program = ShaderUtils.createProgram(vertexShader, fragmentShader);
        if (program > 0) {
            // 9.得到着色器中的属性
            av_position = GLES20.glGetAttribLocation(program, "av_Position");
            af_potion = GLES20.glGetUniformLocation(program, "af_Position");
        }
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        // 清屏
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        // 白色清屏
        GLES20.glClearColor(1.0f, 1.0f, 1.0f, 1);

        // 10.使用源程序
        GLES20.glUseProgram(program);
        // 11.使顶点属性数组有效
        GLES20.glEnableVertexAttribArray(av_position);
        // size 表示坐标是几个参数(x,y 是2; x,y,z 是3)
        // normalized 是否强制转换到 -1 —— 1 之间， false 使用真实的坐标  true 强制转换到-1 ——— 1
        // stride 点的大小，单位字节 （1个点包括2个值, float 是4个字节，占用8个字节）
        // vertexBuffer 内存空间
        GLES20.glVertexAttribPointer(av_position, 2, GLES20.GL_FLOAT, false, 8, vertexBuffer);
        // 12.片元着色器赋值 代表红绿蓝，透明度的归一化值 x:red y:green z:blue w:alpha
        GLES20.glUniform4f(af_potion, 0, 0, 1, 1f);
        if (quadrilateralType == 0) {
            // 13.绘制三角形
            GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, 3);
        } else if (quadrilateralType == 1) {
            // 13.绘制四边形
            GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, 6);
        } else if (quadrilateralType == 2) {
            // 13.绘制四边形
//            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
            // 13.绘制三角形
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 1, 3);
        }
    }
}
