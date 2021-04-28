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
    private String TAG = MainActivity.this.getClass().getSimpleName();
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
        TextView tv_paly_resume = findViewById(R.id.tv_paly_resume);
        TextView tv_paly_pause = findViewById(R.id.tv_paly_pause);
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

        tv_paly_resume.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.callOnResume();
            }
        });

        tv_paly_pause.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.callOnPause();
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

                File file = new File(getCacheDir(), "test.pcm");
                if (file.exists()) {
                    pcmFilePath = file.getAbsolutePath();
                    LogUtils.d("ffmpeg: file ", filePath + "\r\n pcmFilePath: " + pcmFilePath);
                } else {
                    pcmFilePath = FileUtils.copyAssetAndWrite(MainActivity.this, "mydream.pcm");
                }
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

            @Override
            public void onLoad(boolean isLoad) {
//                if (isLoad) {
//                    LogUtils.d("ffmpeg: 加载中");
//                } else {
//                    LogUtils.d("ffmpeg: 播放中");
//                }
            }

            @Override
            public void onPause() {
                LogUtils.d(TAG, "ffmpeg: 暂停");
            }

            @Override
            public void onResume() {
                LogUtils.d(TAG, "ffmpeg: 播放");
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
        //       ` jniSdk.callOnLoad(true);`
        jniSdk.setSource(filePath);
        jniSdk.parpared();
    }

}