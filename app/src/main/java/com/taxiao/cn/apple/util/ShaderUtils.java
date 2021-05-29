package com.taxiao.cn.apple.util;

import android.content.Context;
import android.opengl.GLES20;


import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

/**
 * 顶点着色器 和 片元着色器 加载
 * Created by hanqq on 2021/5/29
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class ShaderUtils {

    private static String TAG = "ShaderUtils";

    /**
     * 将着色器文件加载成 String 类型
     *
     * @param context
     * @param rawId
     * @return
     */
    public static String readRawTxt(Context context, int rawId) {
        if (context == null) return null;
        InputStream inputStream = context.getResources().openRawResource(rawId);
        BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(inputStream));
        StringBuffer stringBuffer = new StringBuffer();
        String line;
        try {
            while ((line = bufferedReader.readLine()) != null) {
                stringBuffer.append(line).append("\n");
            }
            bufferedReader.close();
            bufferedReader = null;
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (bufferedReader != null) {
                try {
                    bufferedReader.close();
                    bufferedReader = null;
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return stringBuffer.toString();
    }

    public static int loadShader(int shaderType, String source) {
        // 1.创建shader
        int shader = GLES20.glCreateShader(shaderType);
        if (shader != 0) {
            // 2.加载shader资源
            GLES20.glShaderSource(shader, source);
            // 3.编译shader
            GLES20.glCompileShader(shader);
            // 4.检查是否编译成功
            int[] compileStatus = new int[1];
            GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compileStatus, 0);
            if (compileStatus[0] != GLES20.GL_TRUE) {
                LogUtils.d(TAG, "shader 编译失败");
                GLES20.glDeleteShader(shader);
                shader = 0;
            }
        }
        return shader;
    }

    /**
     * 创建渲染程序
     *
     * @param vertexSource   顶点着色器
     * @param fragmentSource 片元着色器
     * @return
     */
    public static int createProgram(String vertexSource, String fragmentSource) {
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, vertexSource);
        if (vertexShader == 0) {
            return 0;
        }
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource);
        if (fragmentShader == 0) {
            return 0;
        }
        // 5.创建渲染程序
        int createProgram = GLES20.glCreateProgram();
        if (createProgram != 0) {
            // 6.将着色器程序添加到渲染程序中
            GLES20.glAttachShader(createProgram, vertexShader);
            GLES20.glAttachShader(createProgram, fragmentShader);
            // 7.链接源程序
            GLES20.glLinkProgram(createProgram);
            // 8.检查链接源程序是否成功
            int[] linkStatus = new int[1];
            GLES20.glGetProgramiv(createProgram, GLES20.GL_LINK_STATUS, linkStatus, 0);
            if (linkStatus[0] != GLES20.GL_TRUE) {
                LogUtils.d(TAG, "program 链接失败");
                GLES20.glDeleteProgram(createProgram);
                createProgram = 0;
            }
        }
        return createProgram;
    }
}
