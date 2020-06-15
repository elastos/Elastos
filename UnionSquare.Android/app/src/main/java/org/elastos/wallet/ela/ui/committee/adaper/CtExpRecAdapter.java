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

package org.elastos.wallet.ela.ui.committee.adaper;

import android.content.Context;
import android.graphics.Color;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.committee.bean.CtDetailBean;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.DateUtil;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * Performance record
 */
public class CtExpRecAdapter extends RecyclerView.Adapter<CtExpRecAdapter.ViewHolder> {

    public CtExpRecAdapter(Context context, List<CtDetailBean.Term> list) {
        this.context = context;
        this.list = list;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_ct_detail_experience, viewGroup, false);
        return new ViewHolder(v);
    }

    private static final String VOTING = "VOTING"; //委员评议
    private static final String NOTIFICATION = "NOTIFICATION"; //公示中
    private static final String ACTIVE = "ACTIVE"; //执行中
    private static final String FINAL = "FINAL"; //已完成
    private static final String REJECTED = "REJECTED"; //已废止

    private static final String SUPPORT = "support"; //赞同
    private static final String REJECT = "reject"; //反对
    private static final String ABSTENTION = "abstention"; //弃权
    private static final String UNDECIDED = "undecided"; //为操作

    @Override
    public void onBindViewHolder(@NonNull ViewHolder viewHolder, int i) {
        CtDetailBean.Term data = list.get(i);
        viewHolder.title.setText(data.getTitle());
        String status = data.getStatus();
        String statusStr = "";
        if(!AppUtlis.isNullOrEmpty(status)) {
            switch (status) {
                case VOTING:
                    statusStr = context.getString(R.string.proposalcomments);
                    break;
                case NOTIFICATION:
                    statusStr = context.getString(R.string.proposalpublished);
                    break;
                case ACTIVE:
                    statusStr = context.getString(R.string.proposalprocess);
                    break;
                case FINAL:
                    statusStr = context.getString(R.string.proposalfinish);
                    break;
                case REJECTED:
                    statusStr = context.getString(R.string.proposalabandon);
                    break;
                default:
                    statusStr = "";
            }
        }
        viewHolder.subTitle.setText(String.format("#%1$d %2$s %3$s %4$s",
                data.getId(),
                DateUtil.formatTimestamp(String.valueOf(0==data.getCreatedAt()?"":data.getCreatedAt()), "yyyy.MM.dd"),
                AppUtlis.isNullOrEmpty(data.getDidName())?"":data.getDidName(),
                AppUtlis.isNullOrEmpty(statusStr)?"":statusStr));
        if(data.getVoteResult().equalsIgnoreCase(ABSTENTION)) {
            viewHolder.tag.setBackground(context.getDrawable(R.drawable.ct_abstention_tag_border));
            viewHolder.tag.setText(context.getString(R.string.abstention));
        } else if(data.getVoteResult().equalsIgnoreCase(REJECT)) {
            viewHolder.tag.setBackground(context.getDrawable(R.drawable.ct_reject_tag_border));
            viewHolder.tag.setText(context.getString(R.string.disagree1));
        } else if(data.getVoteResult().equalsIgnoreCase(SUPPORT)) {
            viewHolder.tag.setBackground(context.getDrawable(R.drawable.ct_support_tag_border));
            viewHolder.tag.setText(context.getString(R.string.agree1));
        } else {
            viewHolder.tag.setText("");
            viewHolder.tag.setBackgroundColor(Color.parseColor("#00000000"));
        }

        if (commonRvListener != null) {
            viewHolder.itemView.setOnClickListener(v -> commonRvListener.onRvItemClick(i, data));
        }
    }

    @Override
    public int getItemCount() {
        return list==null ? 0: list.size();
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        @BindView(R.id.title)
        TextView title;
        @BindView(R.id.sub_title)
        TextView subTitle;
        @BindView(R.id.tag)
        TextView tag;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private Context context;
    private CommonRvListener commonRvListener;
    private List<CtDetailBean.Term> list;
}
