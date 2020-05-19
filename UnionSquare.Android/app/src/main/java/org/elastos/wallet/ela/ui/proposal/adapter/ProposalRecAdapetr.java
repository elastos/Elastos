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
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalSearchEntity;
import org.elastos.wallet.ela.utils.DateUtil;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;


public class ProposalRecAdapetr extends RecyclerView.Adapter<ProposalRecAdapetr.ViewHolder> {


    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private CommonRvListener commonRvListener;
    private List<ProposalSearchEntity.DataBean.ListBean> list;

    private Context context;


    public ProposalRecAdapetr(Context context, List<ProposalSearchEntity.DataBean.ListBean> list) {
        this.list = list;
        this.context = context;


    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_propasal_item, parent, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(ProposalRecAdapetr.ViewHolder holder, final int position) {
        ProposalSearchEntity.DataBean.ListBean bean = list.get(position);
        holder.tvTitle.setText(bean.getTitle());
        holder.tvNum.setText("#"+bean.getId() + "");
        holder.tvTime.setText(DateUtil.timeNYR(bean.getCreatedAt(), context, true));
        holder.tvPeople.setText(bean.getProposedBy());
        String status = bean.getStatus();
        switch (status) {
            case "VOTING":
                //委员评议
                status = context.getString(R.string.menberreview);
                break;
            case "NOTIFICATION":

                status = context.getString(R.string.publishing);
                break;
            case "ACTIVE":

                status = context.getString(R.string.executing);
                break;
            case "FINAL":

                status = context.getString(R.string.finish);
                break;
            case "REJECTED":

                status = context.getString(R.string.nopass);
                break;
            case "VETOED":

                status = context.getString(R.string.hasreject);
                break;
        }

        holder.tvStatus.setText(status);
        if (commonRvListener != null) {
            holder.itemView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    commonRvListener.onRvItemClick(position, bean);
                }
            });
        }
    }

    @Override
    public int getItemCount() {
        return list.size();
    }


    public static class ViewHolder extends RecyclerView.ViewHolder {
        @BindView(R.id.tv_num)
        TextView tvNum;
        @BindView(R.id.tv_title)
        TextView tvTitle;
        @BindView(R.id.tv_time)
        TextView tvTime;
        @BindView(R.id.tv_people)
        TextView tvPeople;
        @BindView(R.id.tv_status)
        TextView tvStatus;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }


}
