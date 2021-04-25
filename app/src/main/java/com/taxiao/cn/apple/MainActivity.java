package com.taxiao.cn.apple;

import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.taxiao.ffmpeg.JniSdkImpl;
import com.taxiao.ffmpeg.utils.Function;
import com.taxiao.ffmpeg.utils.IFFmpegParparedListener;
import com.taxiao.ffmpeg.utils.XXPermissionsUtils;

import java.io.File;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class MainActivity extends AppCompatActivity {

    private JniSdkImpl jniSdk;

    private String filePath;
    private String pcmFilePath;
    private ExecutorService executorService;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TextView tv = findViewById(R.id.sample_text);
        TextView tv_start = findViewById(R.id.tv_start);
        TextView tv_thread = findViewById(R.id.tv_thread);
        TextView tv_call_main_back = findViewById(R.id.tv_call_main_back);
        TextView tv_call_thread_back = findViewById(R.id.tv_call_thread_back);
        TextView tv_call_ffmpeg_codec = findViewById(R.id.tv_call_ffmpeg_codec);
        TextView tv_call_ffmpeg_codec_init = findViewById(R.id.tv_call_ffmpeg_codec_init);
        TextView tv_paly_pcm = findViewById(R.id.tv_paly_pcm);
        jniSdk = new JniSdkImpl();

        tv_start.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.mutexThread();
            }
        });

        tv_thread.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.startThread();
            }
        });

        tv_call_main_back.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.javaMain2C();
            }
        });

        tv_call_thread_back.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.javaThread2C();
            }
        });

        tv_call_ffmpeg_codec.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.testFFmpeg();
            }
        });

        tv_call_ffmpeg_codec_init.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                parpared();
            }
        });

        tv_paly_pcm.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.playPcm(pcmFilePath);
            }
        });

        jniSdk.setOnCallBack(new JniSdkImpl.MyCallBack() {
            @Override
            public void error(int code, String name) {
                LogUtils.d("callback ", String.format("code=%s, name=%s", code, name));
            }

            @Override
            public void success() {

            }
        });

        executorService = Executors.newSingleThreadExecutor();

        executorService.execute(new Runnable() {
            @Override
            public void run() {
                filePath = FileUtils.copyAssetAndWrite(MainActivity.this, "meetyou.mp3");
//                pcmFilePath = FileUtils.copyAssetAndWrite(MainActivity.this, "mydream.pcm");
                // meetyou.mp3 重采样文件 test.pcm 播放
                pcmFilePath = new File(getCacheDir(), "test.pcm").getAbsolutePath();
                LogUtils.d("ffmpeg: file ", filePath + "\r\n pcmFilePath: " + pcmFilePath);
            }
        });

        jniSdk.setIFFmpegParparedListener(new IFFmpegParparedListener() {
            @Override
            public void parpared() {
                LogUtils.d("ffmpeg: parpared ");
                jniSdk.start();
            }

            @Override
            public void start() {

            }

            @Override
            public void stop() {

            }
        });
        XXPermissionsUtils.getInstances().hasReadAndWritePermission(new Function<Boolean>() {
            @Override
            public void action(Boolean var) {

            }
        }, MainActivity.this);
    }

    public void parpared() {
        // https 的链接端口无法识别
        jniSdk.setSource(filePath);
        jniSdk.parpared();
    }

}