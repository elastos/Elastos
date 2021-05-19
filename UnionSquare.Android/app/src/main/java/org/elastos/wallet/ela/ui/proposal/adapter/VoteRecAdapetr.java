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
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalDetailEntity;
import org.elastos.wallet.ela.utils.svg.GlideApp;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;


public class VoteRecAdapetr extends RecyclerView.Adapter<VoteRecAdapetr.ViewHolder> {


    private List<ProposalDetailEntity.DataBean.VoteResultBean> list;

    private Context context;


    public VoteRecAdapetr(Context context, List<ProposalDetailEntity.DataBean.VoteResultBean> list) {
        this.list = list;
        this.context = context;


    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_propasal_vote, parent, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(VoteRecAdapetr.ViewHolder holder, final int position) {
        ProposalDetailEntity.DataBean.VoteResultBean bean = list.get(position);
        GlideApp.with(context).load(bean.getAvatar()).error(R.mipmap.found_vote_initial_circle).circleCrop().into(holder.ivIcon);
        holder.tvName.setText(bean.getVotedBy());
        holder.tvDescription.setText(bean.getReason());


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
        @BindView(R.id.tv_description)
        TextView tvDescription;

        ViewHolder(View view) {

            super(view);
            ButterKnife.bind(this, view);
        }
    }


}
