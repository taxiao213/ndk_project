package com.taxiao.cn.apple;

import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.Surface;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.taxiao.cn.apple.model.Paramter;
import com.taxiao.cn.apple.opengl.TXGLSurfaceImageView;
import com.taxiao.cn.apple.opengl.TXGLSurfaceVideoView;
import com.taxiao.cn.apple.util.DialogUtils;
import com.taxiao.cn.apple.util.FileUtils;
import com.taxiao.cn.apple.util.LogUtils;
import com.taxiao.ffmpeg.JniSdkImpl;
import com.taxiao.ffmpeg.utils.Function;
import com.taxiao.ffmpeg.utils.IFFmpegCompleteListener;
import com.taxiao.ffmpeg.utils.IFFmpegCutAudioListener;
import com.taxiao.ffmpeg.utils.IFFmpegDecodeVideoListener;
import com.taxiao.ffmpeg.utils.IFFmpegErrorListener;
import com.taxiao.ffmpeg.utils.IFFmpegGLSurfaceListener;
import com.taxiao.ffmpeg.utils.IFFmpegParparedListener;
import com.taxiao.ffmpeg.utils.IFFmpegRecordTimeListener;
import com.taxiao.ffmpeg.utils.IFFmpegTimeListener;
import com.taxiao.ffmpeg.utils.IFFmpegValumeDBListener;
import com.taxiao.ffmpeg.utils.TXConstant;
import com.taxiao.ffmpeg.utils.TimeInfoModel;
import com.taxiao.ffmpeg.utils.XXPermissionsUtils;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * 将音乐文件放到 assets 目录下,支持.ape , .wav , .mp3 , .flac , .aac 等格式
 */
public class MainActivity extends AppCompatActivity {
    private String TAG = MainActivity.this.getClass().getSimpleName();
    private TextView tv_time_call;
    private TextView tv_source_change;
    private TextView tv_call_error;
    private TextView tv_pitch;
    private TextView tv_speed;
    private TextView tv_valume;
    private TextView tv_valume_db;
    private TextView tv_record_time;
    private SeekBar seekbar_time;

    private JniSdkImpl jniSdk;
    private ExecutorService executorService;
    private String filePath;
    private String pcmFilePath;
    private int seekProgress;
    private int volumeProgress = Constant.VOLUME_PROGRESS;
    private int channel_mute = TXConstant.CHANNEL_STEREO;
    private float pitchProgress = Constant.INIT_PITCH_COEFFICIENT;
    private float speedProgress = Constant.INIT_SPEED_COEFFICIENT;
    private ArrayList<Paramter> musicList = new ArrayList<>();
    private Function<Paramter> function;
    private DialogUtils dialogUtils;
    private boolean isSeek = false;
    private TXGLSurfaceVideoView gles_video;

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
        seekbar_time = findViewById(R.id.seekbar_time);

        TextView tv_stereo = findViewById(R.id.tv_stereo);
        TextView tv_left_channel = findViewById(R.id.tv_left_channel);
        TextView tv_right_channel = findViewById(R.id.tv_right_channel);
        SeekBar seekbar_pitch = findViewById(R.id.seekbar_pitch);// 变调
        SeekBar seekbar_speed = findViewById(R.id.seekbar_speed);// 变速
        tv_pitch = findViewById(R.id.tv_pitch);
        tv_speed = findViewById(R.id.tv_speed);
        tv_valume = findViewById(R.id.tv_valume);
        tv_valume_db = findViewById(R.id.tv_valume_db);

        TextView tv_audio_start = findViewById(R.id.tv_audio_start);
        TextView tv_audio_pause = findViewById(R.id.tv_audio_pause);
        TextView tv_audio_resume = findViewById(R.id.tv_audio_resume);
        TextView tv_audio_stop = findViewById(R.id.tv_audio_stop);
        tv_record_time = findViewById(R.id.tv_record_time);
        TextView tv_cut = findViewById(R.id.tv_cut);
        TextView tv_opengles = findViewById(R.id.tv_opengles);
        gles_video = findViewById(R.id.gles_video);
        TextView tv_opengles_image = findViewById(R.id.tv_opengles_image);

        seekbar_pitch.setProgress((int) (pitchProgress * Constant.PITCH_COEFFICIENT));
        seekbar_speed.setProgress((int) (speedProgress * Constant.SPEED_COEFFICIENT));
        seekbar_volume.setProgress(volumeProgress);
        tv_valume.setText(getString(R.string.audio_valume, String.valueOf(volumeProgress)));
        tv_pitch.setText(getString(R.string.audio_pitch, String.valueOf(pitchProgress)));
        tv_speed.setText(getString(R.string.audio_speed, String.valueOf(speedProgress)));
        tv_record_time.setText(getString(R.string.audio_record, String.valueOf(0)));

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
                dialogUtils = new DialogUtils(MainActivity.this).setVideoFunction(musicList, function);
            }
        });

        tv_audio_start.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                File file = new File(getCacheDir(), "record.aac");
                jniSdk.startRecord(file);
            }
        });

        tv_audio_pause.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.resumeRecord(false);
            }
        });

        tv_audio_resume.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.resumeRecord(true);
            }
        });

        tv_audio_stop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                jniSdk.stopRecord();
            }
        });

        tv_cut.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                File cutFile = new File(getCacheDir(), "cut.pcm");
                jniSdk.cutAudio(Constant.CUT_START_TIME, Constant.CUT_END_TIME, true, cutFile.getAbsolutePath());
            }
        });

        tv_opengles.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity(new Intent(MainActivity.this, OpenGLESActivity.class));
            }
        });

        tv_opengles_image.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity(new Intent(MainActivity.this, OpenGLESImageActivity.class));
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
                LogUtils.d(TAG, "seekbar_time onStartTrackingTouch");
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                LogUtils.d(TAG, "seekbar_time onStopTrackingTouch");
                jniSdk.n_seek(seekProgress);
            }
        });

        seekbar_volume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                LogUtils.d(TAG, "seekbar_volume process : " + progress);
                volumeProgress = progress;
                tv_valume.setText(getString(R.string.audio_valume, String.valueOf(volumeProgress)));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                jniSdk.n_volume(volumeProgress);
            }
        });

        seekbar_pitch.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                LogUtils.d(TAG, "seekbar_pitch process : " + progress);
                pitchProgress = progress / Constant.PITCH_COEFFICIENT;
                tv_pitch.setText(getString(R.string.audio_pitch, String.valueOf(pitchProgress)));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                jniSdk.setPitch(pitchProgress);
            }
        });

        seekbar_speed.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                LogUtils.d(TAG, "seekbar_speed process : " + progress);
                speedProgress = progress / Constant.SPEED_COEFFICIENT;
                tv_speed.setText(getString(R.string.audio_speed, String.valueOf(speedProgress)));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                jniSdk.setSpeed(speedProgress);
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
                // todo 将音乐文件放到 assets 目录下,支持.ape , .wav , .mp3 , .flac , .aac 等格式
                musicList.add(new Paramter(Constant.MUSIC_NAME12, Constant.MUSIC_FILE_NAME12, FileUtils.copyAssetAndWrite(MainActivity.this, Constant.MUSIC_FILE_NAME12)));
//                musicList.add(new Paramter(Constant.MUSIC_NAME10, Constant.MUSIC_FILE_NAME10,  "/storage/9016-4EF8/axure.mp4"));
                musicList.add(new Paramter(Constant.MUSIC_NAME10, Constant.MUSIC_FILE_NAME10, "/storage/9016-4EF8/Android网易/Android音视频/13.OpenGL ES 绘制流程 着色器.mp4"));
                musicList.add(new Paramter(Constant.MUSIC_NAME10, Constant.MUSIC_FILE_NAME10, "/storage/9016-4EF8/英语二/谢孟媛/帮謝孟媛老師PO初級文法第01集.mp4"));
                musicList.add(new Paramter(Constant.MUSIC_NAME10, Constant.MUSIC_FILE_NAME10, FileUtils.copyAssetAndWrite(MainActivity.this, Constant.MUSIC_FILE_NAME10)));
                musicList.add(new Paramter(Constant.MUSIC_NAME11, Constant.MUSIC_FILE_NAME11, FileUtils.copyAssetAndWrite(MainActivity.this, Constant.MUSIC_FILE_NAME11)));
//                musicList.add(new Paramter(Constant.MUSIC_NAME1, Constant.MUSIC_FILE_NAME1, FileUtils.copyAssetAndWrite(MainActivity.this, Constant.MUSIC_FILE_NAME1)));
//                musicList.add(new Paramter(Constant.MUSIC_NAME2, Constant.MUSIC_FILE_NAME2, FileUtils.copyAssetAndWrite(MainActivity.this, Constant.MUSIC_FILE_NAME2)));
//                musicList.add(new Paramter(Constant.MUSIC_NAME3, Constant.MUSIC_FILE_NAME3, FileUtils.copyAssetAndWrite(MainActivity.this, Constant.MUSIC_FILE_NAME3)));
//                musicList.add(new Paramter(Constant.MUSIC_NAME4, Constant.MUSIC_FILE_NAME4, FileUtils.copyAssetAndWrite(MainActivity.this, Constant.MUSIC_FILE_NAME4)));
//                musicList.add(new Paramter(Constant.MUSIC_NAME5, Constant.MUSIC_FILE_NAME5, FileUtils.copyAssetAndWrite(MainActivity.this, Constant.MUSIC_FILE_NAME5)));
//                musicList.add(new Paramter(Constant.MUSIC_NAME6, Constant.MUSIC_FILE_NAME6, FileUtils.copyAssetAndWrite(MainActivity.this, Constant.MUSIC_FILE_NAME6)));
//                musicList.add(new Paramter(Constant.MUSIC_NAME7, Constant.MUSIC_FILE_NAME7, FileUtils.copyAssetAndWrite(MainActivity.this, Constant.MUSIC_FILE_NAME7)));
//                musicList.add(new Paramter(Constant.MUSIC_NAME9, Constant.MUSIC_FILE_NAME9, FileUtils.copyAssetAndWrite(MainActivity.this, Constant.MUSIC_FILE_NAME9)));
                musicList.add(new Paramter(Constant.MUSIC_NAME8, Constant.MUSIC_FILE_NAME8, FileUtils.copyAssetAndWrite(MainActivity.this, Constant.MUSIC_FILE_NAME8)));
                musicList.add(new Paramter(Constant.MUSIC_FILE_NET, Constant.MUSIC_FILE_NAME_NET, Constant.MUSIC_FILE_PATH_NET));
                filePath = musicList.get(0).getPath();
//                pcmFilePath = FileUtils.copyAssetAndWrite(MainActivity.this, "mydream.pcm");
                // meetyou.mp3 重采样文件 test.pcm 播放

                File file = new File(getCacheDir(), "test.pcm");
                if (file.exists()) {
                    pcmFilePath = file.getAbsolutePath();
                    LogUtils.d("ffmpeg: file ", filePath + "\r\n pcmFilePath: " + pcmFilePath);
                } else {
                    pcmFilePath = FileUtils.copyAssetAndWrite(MainActivity.this, Constant.MUSIC_FILE_NAME7);
                }
            }
        });

        jniSdk.setIFFmpegParparedListener(new IFFmpegParparedListener() {
            @Override
            public void parpared() {
                LogUtils.d("ffmpeg: parpared ");
                jniSdk.setVolume(volumeProgress);
                jniSdk.setMute(channel_mute);
                jniSdk.setPitch(pitchProgress);
                jniSdk.setSpeed(speedProgress);
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
                if (isLoad) {
                    LogUtils.d("ffmpeg: 加载中");
                } else {
                    LogUtils.d("ffmpeg: 播放中");
                }
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
//                        seekbar_time.setProgress((int) (timeInfoModel.getCurrentTime() * 100.0f / timeInfoModel.getTotalTime()));
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

        jniSdk.setIFFmpegValumeDBListener(new IFFmpegValumeDBListener() {
            @Override
            public void onDbValue(final int db) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        tv_valume_db.setText(getString(R.string.audio_valume_db, String.valueOf(db)));
                    }
                });
            }
        });

        jniSdk.setIFFmpegRecordTimeListener(new IFFmpegRecordTimeListener() {
            @Override
            public void onRecordTime(final float time) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        tv_record_time.setText(getString(R.string.audio_record, String.valueOf((int) time)));
                    }
                });
            }
        });

        jniSdk.setIFFmpegCutAudioListener(new IFFmpegCutAudioListener() {
            @Override
            public void cutAudio(int sampleRate, byte[] buffer) {
                LogUtils.d(TAG, "cutAudio sampleRate: " + sampleRate);
                // 将返回的 pcm 数据存储
            }
        });

        jniSdk.setIFFmpegDecodeVideoListener(new IFFmpegDecodeVideoListener() {
            @Override
            public void onRenderYUV(int width, int height, byte[] y, byte[] u, byte[] v) {
                if (gles_video != null) {
                    gles_video.setYUVData(width, height, y, u, v);
                }
            }

            @Override
            public void renderType(boolean renderYUV) {
                if (gles_video != null) {
                    gles_video.setRenderType(renderYUV);
                }
            }
        });

        jniSdk.setFFmpegGLSurfaceListener(new IFFmpegGLSurfaceListener() {
            @Override
            public Surface getSurface() {
                if (gles_video != null) {
                    return gles_video.getSurface();
                }
                return null;
            }
        });

        function = new Function<Paramter>() {
            @Override
            public void action(Paramter var1) {
                if (dialogUtils != null) {
                    dialogUtils.onDismiss();
                }
                if (var1 != null) {
                    jniSdk.stopRecord();
                    jniSdk.n_stop();
                    filePath = var1.getPath();
                    parpared();
                }
            }
        };
        XXPermissionsUtils.getInstances().hasReadAndWritePermission(new Function<Boolean>() {
            @Override
            public void action(Boolean var) {

            }
        }, MainActivity.this);
    }

    public void parpared() {
        // https 的链接端口无法识别
        //       ` jniSdk.callOnLoad(true);`
//        String url;
//        if (tv_source_change.isSelected()) {
//            url = Constant.AUDIO_PATH;
//        } else {
//            url = filePath;
//        }
        if (!TextUtils.isEmpty(filePath)) {
            jniSdk.setSource(filePath);
            jniSdk.parpared();
        }
    }

}