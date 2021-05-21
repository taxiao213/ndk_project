package com.taxiao.cn.apple;

import android.os.Bundle;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.taxiao.ffmpeg.JniSdkImpl;
import com.taxiao.ffmpeg.utils.Function;
import com.taxiao.ffmpeg.utils.IFFmpegCompleteListener;
import com.taxiao.ffmpeg.utils.IFFmpegErrorListener;
import com.taxiao.ffmpeg.utils.IFFmpegParparedListener;
import com.taxiao.ffmpeg.utils.IFFmpegTimeListener;
import com.taxiao.ffmpeg.utils.TXConstant;
import com.taxiao.ffmpeg.utils.TimeInfoModel;
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
    private TextView tv_time_call;
    private TextView tv_source_change;
    private TextView tv_call_error;
    private int seekProgress;
    private int volumeProgress = 50;
    private int channel_mute = TXConstant.CHANNEL_LEFT;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TextView tv_start = findViewById(R.id.tv_start);
        TextView tv_thread = findViewById(R.id.tv_thread);
        TextView tv_call_main_back = findViewById(R.id.tv_call_main_back);
        TextView tv_call_thread_back = findViewById(R.id.tv_call_thread_back);
        TextView tv_call_ffmpeg_codec = findViewById(R.id.tv_call_ffmpeg_codec);
        TextView tv_paly_pcm = findViewById(R.id.tv_paly_pcm);
        TextView tv_play_start = findViewById(R.id.tv_play_start);
        TextView tv_paly_resume = findViewById(R.id.tv_paly_resume);
        TextView tv_paly_pause = findViewById(R.id.tv_paly_pause);
        TextView tv_time_stop = findViewById(R.id.tv_time_stop);
        tv_source_change = findViewById(R.id.tv_source_change);
        tv_time_call = findViewById(R.id.tv_time_call);
        tv_call_error = findViewById(R.id.tv_call_error);
        SeekBar seekbar_volume = findViewById(R.id.seekbar_volume);
        SeekBar seekbar_time = findViewById(R.id.seekbar_time);
        seekbar_volume.setProgress(volumeProgress);
        TextView tv_stereo = findViewById(R.id.tv_stereo);
        TextView tv_left_channel = findViewById(R.id.tv_left_channel);
        TextView tv_right_channel = findViewById(R.id.tv_right_channel);

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

        tv_play_start.setOnClickListener(new View.OnClickListener() {
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

        tv_time_stop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.n_stop();
            }
        });

        tv_stereo.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.n_mute(TXConstant.CHANNEL_STEREO);
            }
        });

        tv_left_channel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.n_mute(TXConstant.CHANNEL_LEFT);
            }
        });

        tv_right_channel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.n_mute(TXConstant.CHANNEL_RIGHT);
            }
        });

        tv_source_change.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // 切换网络和本地播放地址
                tv_source_change.setSelected(!tv_source_change.isSelected());
            }
        });

        seekbar_time.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                LogUtils.d(TAG, "seekbar_time process : " + progress);
                seekProgress = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                jniSdk.n_seek(seekProgress);
            }
        });

        seekbar_volume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                LogUtils.d(TAG, "seekbar_volume process : " + progress);
                volumeProgress = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                jniSdk.n_volume(volumeProgress);
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
                jniSdk.setVolume(volumeProgress);
                jniSdk.setMute(channel_mute);
                jniSdk.n_start();
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

        jniSdk.setIFFmpegTimeListener(new IFFmpegTimeListener() {
            @Override
            public void onTimeInfo(final TimeInfoModel timeInfoModel) {
                LogUtils.d(TAG, "currentTime : " + timeInfoModel.getCurrentTime() + " total: " + timeInfoModel.getTotalTime());
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        // 播放时间
                        tv_time_call.setText(String.format("%d:%d", timeInfoModel.getCurrentTime(), timeInfoModel.getTotalTime()));
                    }
                });
            }
        });

        jniSdk.setIFFmpegErrorListener(
                new IFFmpegErrorListener() {
                    @Override
                    public void error(final int code, final String errorMsg) {
                        LogUtils.d(TAG, "code: " + code + " errorMsg: " + errorMsg);
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                tv_call_error.setText(String.format("code:%d , errormsg:%s", code, errorMsg));
                            }
                        });
                    }
                }
        );

        jniSdk.setIFFmpegCompleteListener(new IFFmpegCompleteListener() {
            @Override
            public void complete() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        tv_time_call.setText("播放完成");
                    }
                });
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
        String url;
        if (tv_source_change.isSelected()) {
            url = Constant.AUDIO_PATH;
        } else {
            url = filePath;
        }
        jniSdk.setSource(url);
        jniSdk.parpared();
    }

}