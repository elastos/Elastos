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

package org.elastos.wallet.ela.ui.Assets.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.ui.Assets.bean.VoteStatus;
import org.elastos.wallet.ela.utils.Arith;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * 资产页面的rv
 */

public class VoteStatusRecAdapetr extends RecyclerView.Adapter<VoteStatusRecAdapetr.ViewHolder> {


    private List<VoteStatus> list;
    private Context context;


    public VoteStatusRecAdapetr(Context context, List<VoteStatus> list) {
        this.context = context;
        this.list = list;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_votestatus, parent, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        VoteStatus voteStatus = list.get(position);
        holder.ivIcon.setImageResource(voteStatus.getIconID());
        holder.tvCount.setText(Arith.div(voteStatus.getCount(), MyWallet.RATE_S, 8).longValue() + " ELA");
        holder.tvName.setText(voteStatus.getName());
        switch (voteStatus.getStatus()) {
            //0没有投票   1 有投票部分失效 2 有投票完全失效 3有投票无失效
            case 0:
                holder.itemView.setAlpha(0.5f);
                holder.tvCount.setText("--");
                break;
            case 1:
                holder.tvStutus.setVisibility(View.VISIBLE);
                holder.tvStutus.setText(R.string.partineffect);
                break;
            case 2:
                holder.tvStutus.setVisibility(View.VISIBLE);
                holder.tvStutus.setText(R.string.expired);
                break;

        }

    }

    @Override
    public int getItemCount() {
        return list.size();
    }


    public static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.iv_icon)
        ImageView ivIcon;
        @BindView(R.id.tv_stutus)
        TextView tvStutus;
        @BindView(R.id.tv_count)
        TextView tvCount;
        @BindView(R.id.tv_name)
        TextView tvName;


        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

}
