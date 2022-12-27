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

package org.elastos.wallet.ela.ui.Assets.adapter.votestatus;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.ui.committee.bean.CtListBean;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalSearchEntity;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalDetailPresenter;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.NumberiUtil;

import java.math.BigDecimal;
import java.util.ArrayList;

import butterknife.BindView;
import butterknife.ButterKnife;


/**
 * Created by wangdongfeng on 2018/4/14.
 */

public class DeposRecAdapetr extends RecyclerView.Adapter<DeposRecAdapetr.ViewHolderParent> {

    private CommonRvListener1 commonRvListener;
    private String type;

    private Context context;
    ArrayList<Object> data;

    public DeposRecAdapetr(Context context, String type, ArrayList<Object> data) {
        this.type = type;
        this.context = context;
        this.data = data;
    }

    public void setCommonRvListener(CommonRvListener1 commonRvListener) {
        this.commonRvListener = commonRvListener;
    }


    @Override
    public ViewHolderParent onCreateViewHolder(ViewGroup parent, int viewType) {

        ViewHolderParent holder = new ViewHolderParent(new TextView(context));
        switch (type) {
            case "Delegate":
            case "CRC":

                holder = new ViewHolderDepos(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_myvoteafragment, parent, false));
                break;
            case "CRCImpeachment":
                holder = new ViewHolderImpeach(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_impeach, parent, false));


                break;
            case "CRCProposal":
                holder = new ViewHolderProposal(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_proposal, parent, false));

                break;

        }


        return holder;
    }

    @Override
    public void onBindViewHolder(ViewHolderParent holder, int position) {

        switch (type) {
            case "Delegate":
                VoteListBean.DataBean.ResultBean.ProducersBean producersBean = (VoteListBean.DataBean.ResultBean.ProducersBean) data.get(position);
                if ("Active".equals(producersBean.getState())) {
                    ((ViewHolderDepos) holder).itemView.setAlpha(1);
                    ((ViewHolderDepos) holder).tvName.setText(producersBean.getNickname());
                    ((ViewHolderDepos) holder).tvNo.setText("No." + (producersBean.getIndex() + 1));
                } else {
                    ((ViewHolderDepos) holder).itemView.setAlpha(0.5f);
                    ((ViewHolderDepos) holder).tvName.setText(context.getString(R.string.invalidcr));
                    ((ViewHolderDepos) holder).tvNo.setText("--");
                }
                break;

            case "CRC":
                CRListBean.DataBean.ResultBean.CrcandidatesinfoBean crcBean = (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean) data.get(position);
                ((ViewHolderDepos) holder).tvNo.setText(NumberiUtil.removeZero(Arith.div(crcBean.getVotes(), MyWallet.RATE_S, 8).toPlainString()));
                if ("Active".equals(crcBean.getState())) {
                    ((ViewHolderDepos) holder).itemView.setAlpha(1);
                    ((ViewHolderDepos) holder).tvName.setText("No." + (crcBean.getIndex() + 1) + crcBean.getNickname());
                } else {
                    ((ViewHolderDepos) holder).itemView.setAlpha(0.5f);
                    ((ViewHolderDepos) holder).tvName.setText(context.getString(R.string.invalidcr));
                }
                break;
            case "CRCImpeachment":
                CtListBean.Council council = (CtListBean.Council) data.get(position);
                ((ViewHolderImpeach) holder).tvName.setText(TextUtils.isEmpty(council.getDidName()) ? context.getString(R.string.impeachinefect) : council.getDidName());
                ((ViewHolderImpeach) holder).tvCount.setText(NumberiUtil.removeZero(Arith.div(council.getVotes(), MyWallet.RATE_S, 8).toPlainString()));
                if ("Elected".equals(council.getStatus())) {
                    ((ViewHolderImpeach) holder).tvStatus.setVisibility(View.GONE);
                    String text = new BigDecimal(council.getRejectRatio()).multiply(new BigDecimal(100)).setScale(2, BigDecimal.ROUND_DOWN).floatValue() + "%";
                    ((ViewHolderImpeach) holder).tvPercent.setText(text);
                } else {
                    ((ViewHolderImpeach) holder).itemView.setAlpha(0.5f);
                    ((ViewHolderImpeach) holder).tvStatus.setVisibility(View.VISIBLE);
                    ((ViewHolderImpeach) holder).tvPercent.setText("--");
                }

                break;
            case "CRCProposal":
                ProposalSearchEntity.DataBean.ListBean listBean = (ProposalSearchEntity.DataBean.ListBean) data.get(position);
                ((ViewHolderProposal) holder).tvName.setText(TextUtils.isEmpty(listBean.getTitle()) ? "--" : listBean.getTitle());
                ((ViewHolderProposal) holder).tvCount.setText(NumberiUtil.removeZero(Arith.div(listBean.getVotes(), MyWallet.RATE_S, 8).toPlainString()));
                if ("NOTIFICATION".equals(listBean.getStatus())) {
                    ((ViewHolderProposal) holder).itemView.setAlpha(1);
                    ((ViewHolderProposal) holder).tvStatus.setText(context.getString(R.string.resteffecttime) + ":" + ProposalDetailPresenter.setRestDay(listBean.getVoteEndsIn(), context));
                    String text = new BigDecimal(listBean.getRejectRatio()).multiply(new BigDecimal(100)).setScale(2, BigDecimal.ROUND_DOWN).floatValue() + "%";
                    ((ViewHolderProposal) holder).tvPercent.setText(text);
                } else {
                    ((ViewHolderProposal) holder).itemView.setAlpha(0.5f);
                    ((ViewHolderProposal) holder).tvStatus.setText(R.string.expired);
                    ((ViewHolderProposal) holder).tvStatus.setBackgroundResource(R.drawable.sc_white20_cr2_stc_ffffff);
                    ((ViewHolderProposal) holder).tvPercent.setText("--");
                    /*if (TextUtils.isEmpty(listBean.getStatus()))
                        ((ViewHolderImpeach) holder).tvPercent.setText("--");
                    else {
                        String text = new BigDecimal(listBean.getRejectRatio()).multiply(new BigDecimal(100)).setScale(2, BigDecimal.ROUND_DOWN).floatValue() + "%";
                        ((ViewHolderImpeach) holder).tvPercent.setText("100%");
                    }*/
                }

                break;
        }


    }


    @Override
    public int getItemCount() {
        return data.size();
    }


    public static class ViewHolderParent extends RecyclerView.ViewHolder {

        ViewHolderParent(View view) {

            super(view);

        }
    }

    public static class ViewHolderDepos extends ViewHolderParent {
        @BindView(R.id.tv_name)
        TextView tvName;
        @BindView(R.id.tv_no)
        TextView tvNo;

        ViewHolderDepos(View view) {

            super(view);
            ButterKnife.bind(this, view);
        }
    }

    public static class ViewHolderImpeach extends ViewHolderParent {

        @BindView(R.id.tv_name)
        TextView tvName;
        @BindView(R.id.tv_count)
        TextView tvCount;
        @BindView(R.id.tv_percent)
        TextView tvPercent;
        @BindView(R.id.tv_status)
        TextView tvStatus;

        ViewHolderImpeach(View view) {

            super(view);
            ButterKnife.bind(this, view);
        }
    }

    public static class ViewHolderProposal extends ViewHolderParent {

        @BindView(R.id.tv_name)
        TextView tvName;
        @BindView(R.id.tv_status)
        TextView tvStatus;
        @BindView(R.id.tv_count)
        TextView tvCount;
        @BindView(R.id.tv_percent)
        TextView tvPercent;

        ViewHolderProposal(View view) {

            super(view);
            ButterKnife.bind(this, view);
        }
    }

}
