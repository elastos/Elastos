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
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecieveProposalJwtEntity;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.SPUtil;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;


public class SuggetMoneyRecAdapetr extends RecyclerView.Adapter<SuggetMoneyRecAdapetr.ViewHolder> {


    private List<RecieveProposalJwtEntity.DataBean.BudgetsBean> list;

    private Context context;


    public SuggetMoneyRecAdapetr(Context context, List<RecieveProposalJwtEntity.DataBean.BudgetsBean> list) {
        this.list = list;
        this.context = context;


    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_propasal_suggest, parent, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(SuggetMoneyRecAdapetr.ViewHolder holder, final int position) {
        RecieveProposalJwtEntity.DataBean.BudgetsBean budgetsBean = list.get(position);
        holder.tvMoney.setText(NumberiUtil.salaToEla(budgetsBean.getAmount()) + " ELA");
        String tag = budgetsBean.getType().toLowerCase();

        switch (tag) {
            case "imprest":
                tag = context.getString(R.string.imprest);
                break;
            case "normalpayment":
                String stage = budgetsBean.getStage() + "";
                int Language = new SPUtil(context).getLanguage();
                if (Language != 0) {
                    switch (budgetsBean.getStage()) {
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

                tag = String.format(context.getString(R.string.thexprocess), stage);
                break;
            case "finalpayment":
                tag = context.getString(R.string.finalpayment);
                break;
        }


        holder.tvTag.setText(tag);

    }

    @Override
    public int getItemCount() {
        return list.size();
    }


    public static class ViewHolder extends RecyclerView.ViewHolder {
        @BindView(R.id.tv_tag)
        TextView tvTag;
        @BindView(R.id.tv_money)
        TextView tvMoney;

        ViewHolder(View view) {

            super(view);
            ButterKnife.bind(this, view);
        }
    }


}
