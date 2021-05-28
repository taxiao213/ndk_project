package com.taxiao.cn.apple.util;

import android.content.Context;
import android.content.DialogInterface;
import android.graphics.drawable.BitmapDrawable;
import android.os.Handler;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.TextView;

import androidx.appcompat.app.AlertDialog;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.taxiao.cn.apple.Constant;
import com.taxiao.cn.apple.R;
import com.taxiao.cn.apple.model.Paramter;
import com.taxiao.ffmpeg.utils.Function;

import java.util.ArrayList;


/**
 * type 1 修改昵称  2 选择相片  4 提示框
 * Created by Han on 2018/8/11
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/yin13753884368
 */

public class DialogUtils {

    private AlertDialog mAlertDialog;
    private Context context;
    private int type;
    private View dialogView;
    private ArrayList<Paramter> mList;
    private Function<Paramter> mFunction;
    private DialogVideoSelectAdapter adapter;

    public DialogUtils(Context context) {
        this.mAlertDialog = new AlertDialog
                .Builder(context)
                .setCancelable(true)
                .create();
        this.context = context;
        mAlertDialog.show();
        init();
    }


    private void init() {
        Window window = mAlertDialog.getWindow();
        window.setBackgroundDrawable(new BitmapDrawable());
        window.setLayout(WindowManager.LayoutParams.WRAP_CONTENT, WindowManager.LayoutParams.WRAP_CONTENT);
        dialogView = LayoutInflater.from(context).inflate(R.layout.dialog_video_select, null);
        window.setContentView(dialogView);
        window.setGravity(Gravity.CENTER);
        RecyclerView ry = window.findViewById(R.id.ry);
        ry.setLayoutManager(new LinearLayoutManager(context));
        mList = new ArrayList<>();
        adapter = new DialogVideoSelectAdapter(context, mList);
        ry.setAdapter(adapter);
    }

    /**
     * 设置点击周边可取消
     *
     * @param cancelable
     * @return
     */
    public DialogUtils setCancelable(boolean cancelable) {
        if (mAlertDialog != null) {
            mAlertDialog.setCancelable(cancelable);
        }
        return this;
    }

    public void onDismiss() {
        if (mAlertDialog != null && mAlertDialog.isShowing()) {
            mAlertDialog.dismiss();
        }
    }

    public DialogUtils setVideoFunction(ArrayList<Paramter> list, Function<Paramter> function) {
        mList.clear();
        mList.addAll(list);
        adapter.setFunction(function);
        adapter.notifyDataSetChanged();
        return this;
    }

}
