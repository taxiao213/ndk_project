package com.taxiao.cn.apple.util;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.taxiao.cn.apple.R;
import com.taxiao.cn.apple.model.Paramter;
import com.taxiao.ffmpeg.utils.Function;

import java.util.List;


/**
 * 视频设置选择弹框
 * Created by hanqq on 2020/7/2
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class DialogVideoSelectAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

    private Context mContext;
    private List<Paramter> list;
    private Function<Paramter> function;

    public DialogVideoSelectAdapter(Context context, List<Paramter> list) {
        this.mContext = context;
        this.list = list;
    }

    @NonNull
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        return new ViewHolder(LayoutInflater.from(mContext).inflate(R.layout.adapter_dialog_video_select, parent, false), list, function);
    }

    @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
        Paramter paramter = list.get(position);
        if (paramter == null) return;
        ViewHolder holder2 = (ViewHolder) holder;
        holder2.tvTitleName.setText(paramter.getName());
    }

    @Override
    public int getItemCount() {
        return list.size();
    }

    public void setFunction(Function<Paramter> function) {
        this.function = function;
    }


    static class ViewHolder extends RecyclerView.ViewHolder {

        TextView tvTitleName;
        View currentView;

        ViewHolder(View view, final List<Paramter> list, final Function<Paramter> function) {
            super(view);
            this.currentView = view;
            tvTitleName = view.findViewById(R.id.tv_title_name);
            currentView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    int adapterPosition = getAdapterPosition();
                    if (function != null) {
                        function.action(list.get(adapterPosition));
                    }
                }
            });
        }
    }
}
