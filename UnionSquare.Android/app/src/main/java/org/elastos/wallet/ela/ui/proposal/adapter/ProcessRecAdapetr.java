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
import android.text.style.ScaleXSpan;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalDetailEntity;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.svg.GlideApp;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;


public class ProcessRecAdapetr extends RecyclerView.Adapter<ProcessRecAdapetr.ViewHolder> {


    private List<ProposalDetailEntity.DataBean.TrackingBean> list;

    private Context context;


    public ProcessRecAdapetr(Context context, List<ProposalDetailEntity.DataBean.TrackingBean> list) {
        this.list = list;
        this.context = context;
        for (int i = 0; i < list.size(); i++) {
            if (list.get(i).getComment() == null || list.get(i).getComment().getOpinion() == null) {
                list.remove(i--);
            }
        }


    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_propasal_process, parent, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(ProcessRecAdapetr.ViewHolder holder, final int position) {

        ProposalDetailEntity.DataBean.TrackingBean bean = list.get(position);

        String stage = bean.getStage() + "";
        int Language = new SPUtil(context).getLanguage();
        if (Language != 0) {
            switch (bean.getStage()) {
                case 1:
                    stage = "1st";
                    break;
                case 2:
                    stage = "2nd";
                    break;
                case 3:
                    stage = "3rd";
                    break;
                default:
                    stage = stage + "th";
            }
        }

        stage = String.format(context.getString(R.string.thexprocess), stage);
        holder.tvStatus.setText(stage);
        GlideApp.with(context).load(bean.getAvatar()).error(R.mipmap.found_vote_initial_circle).circleCrop().into(holder.ivIcon);
        holder.tvName.setText(context.getString(R.string.proposalman) + bean.getDidName());
        holder.tvTime.setText(DateUtil.time(bean.getCreatedAt(), context));
        holder.tvDescription.setText(bean.getContent());

        GlideApp.with(context).load(bean.getComment().getAvatar()).error(R.mipmap.found_vote_initial_circle).circleCrop().into(holder.ivIcon1);
        holder.tvName1.setText(context.getString(R.string.proposalman) + bean.getComment().getCreatedBy());
        holder.tvTime1.setText(DateUtil.time(bean.getComment().getCreatedAt(), context));
        holder.tvDescription1.setText(bean.getComment().getContent());

        switch (bean.getComment().getOpinion().toUpperCase()) {
            // 跟踪审核意见
            //[赞同: 'APPROVED',
            //反对: 'REJECTED']
            case "APPROVED":
                stage = context.getString(R.string.pass);
                break;
            case "REJECTED":
                stage = context.getString(R.string.rejected);
                break;

        }

        String str = stage + " " + bean.getComment().getContent();

        int bend = stage.length();

        SpannableStringBuilder style = new SpannableStringBuilder(str);
        style.setSpan(new ForegroundColorSpan(Color.WHITE), 0, bend, Spannable.SPAN_EXCLUSIVE_INCLUSIVE);
        //style.setSpan(new ScaleXSpan(3.0f),0, bend, Spannable.SPAN_EXCLUSIVE_INCLUSIVE);
        if (stage.equals(context.getString(R.string.pass)))
            style.setSpan(new BackgroundColorSpan(0xff35B08F), 0, bend, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        else
            style.setSpan(new BackgroundColorSpan(0xffB04135), 0, bend, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
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
        @BindView(R.id.tv_status)
        TextView tvStatus;
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
