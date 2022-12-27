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
import org.elastos.wallet.ela.ui.Assets.bean.TransferRecordEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.NumberiUtil;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * 资产页面的rv
 */

public class TransferRecordRecAdapetr extends RecyclerView.Adapter<TransferRecordRecAdapetr.ViewHolder> {


    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private CommonRvListener commonRvListener;
    private List<TransferRecordEntity.TransactionsBean> list;

    private Context context;
    private String chainId;

    public TransferRecordRecAdapetr(Context context, List<TransferRecordEntity.TransactionsBean> list, String chainId) {
        this.list = list;
        this.context = context;
        this.chainId = chainId;

    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_transfer_record, parent, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, final int position) {
        TransferRecordEntity.TransactionsBean data = list.get(position);
        String direction = data.getDirection();//direction有3种, Moved ,Received,Sent
        if (data.getStatus().equals("Pending")) {
            holder.tvStatus.setText(R.string.Pending);
            holder.tvStatus.setTextColor(context.getResources().getColor(R.color.green1));
            holder.tvTime.setText("- -");
        } else {
            holder.tvStatus.setText(R.string.confirmed);
            holder.tvStatus.setTextColor(context.getResources().getColor(R.color.blue1));
            holder.tvTime.setText(DateUtil.time(data.getTimestamp(),context));
        }

        switch (direction) {
            case "Moved":
                //自己给自己
                setViewData(holder, R.mipmap.asset_trade_record_self, NumberiUtil.maxNumberFormat(Arith.div(data.getAmount() + "", MyWallet.RATE_S), 12));
                break;
            case "Received":
                setViewData(holder, R.mipmap.asset_trade_record_in, "+" + NumberiUtil.maxNumberFormat(Arith.div(data.getAmount() + "", MyWallet.RATE_S), 12));
                //接受
                break;
            case "Sent":
            case "Deposit":
                // setViewData(holder, R.mipmap.asset_trade_record_out, "-" + NumberiUtil.maxNumberFormat(((double) data.getAmount() / MyWallet.RATE) + "", 12) + " " + chainId);
                setViewData(holder, R.mipmap.asset_trade_record_out, "-" + NumberiUtil.maxNumberFormat(Arith.div(data.getAmount() + "", MyWallet.RATE_S), 12));
                //发出
                break;

        }
        if (commonRvListener != null) {
            holder.itemView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    commonRvListener.onRvItemClick(position, data);
                }
            });
        }
    }

    @Override
    public int getItemCount() {
        return list.size();
    }


    public static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.iv_status)
        ImageView ivStatus;
        @BindView(R.id.tv_address)
        TextView tvAddress;
        @BindView(R.id.tv_time)
        TextView tvTime;
        @BindView(R.id.tv_balance)
        TextView tvBalance;
        @BindView(R.id.tv_status)
        TextView tvStatus;


        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    private void setViewData(ViewHolder holder, int imageId, String balance) {
        holder.ivStatus.setImageResource(imageId);
        holder.tvBalance.setText(balance);

    }
}
