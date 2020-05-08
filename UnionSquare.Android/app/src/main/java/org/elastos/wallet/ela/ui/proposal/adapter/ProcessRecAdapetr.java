/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.wallet.ela.ui.proposal.adapter;

import android.content.Context;
import android.graphics.Color;
import android.support.v7.widget.RecyclerView;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.style.BackgroundColorSpan;
import android.text.style.ForegroundColorSpan;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;


public class ProcessRecAdapetr extends RecyclerView.Adapter<ProcessRecAdapetr.ViewHolder> {


    private List<String> list;

    private Context context;


    public ProcessRecAdapetr(Context context, List<String> list) {
        this.list = list;
        this.context = context;


    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_propasal_process, parent, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(ProcessRecAdapetr.ViewHolder holder, final int position) {


        String str="这是设置TextView部分文字背景颜色和前景颜色的demo!";
        int bstart=str.indexOf("背景");
        int bend=bstart+"背景".length();
        int fstart=str.indexOf("前景");
        int fend=fstart+"前景".length();
        SpannableStringBuilder style=new SpannableStringBuilder(str);
        style.setSpan(new BackgroundColorSpan(Color.RED),bstart,bend, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        style.setSpan(new BackgroundColorSpan(Color.WHITE),bstart,bend, Spannable.SPAN_EXCLUSIVE_INCLUSIVE);
        holder.tvDescription1.setText(style);

    }

    @Override
    public int getItemCount() {
        return list.size();
    }


    public static class ViewHolder extends RecyclerView.ViewHolder {
        @BindView(R.id.iv_icon)
        ImageView ivIcon;
        @BindView(R.id.tv_name)
        TextView tvName;
        @BindView(R.id.tv_time)
        TextView tvTime;
        @BindView(R.id.tv_description)
        TextView tvDescription;
        @BindView(R.id.iv_icon1)
        ImageView ivIcon1;
        @BindView(R.id.tv_name1)
        TextView tvName1;
        @BindView(R.id.tv_time1)
        TextView tvTime1;
        @BindView(R.id.tv_description1)
        TextView tvDescription1;
        ViewHolder(View view) {

            super(view);
            ButterKnife.bind(this, view);
        }
    }


}
