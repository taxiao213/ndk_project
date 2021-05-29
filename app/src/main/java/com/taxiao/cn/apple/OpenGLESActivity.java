package com.taxiao.cn.apple;

import android.os.Bundle;
import android.view.View;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.taxiao.cn.apple.view.TXGLSurfaceView;

/**
 * Created by hanqq on 2021/5/29
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class OpenGLESActivity extends AppCompatActivity {
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_opengles);
        TXGLSurfaceView gles = findViewById(R.id.gles);
    }
}
