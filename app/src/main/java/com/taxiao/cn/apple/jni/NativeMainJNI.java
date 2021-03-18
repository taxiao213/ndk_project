package com.taxiao.cn.apple.jni;

/**
 * Created by hanqq on 2021/2/8
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class NativeMainJNI {

    static {
        System.loadLibrary("native");
    }

    public static void main(String[] args) {
        new NativeMainJNI().sayHello();
    }
    public native void sayHello();

    public native void start(String fucntion, String json);
}
