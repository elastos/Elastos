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
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.committee.bean.PastCtBean;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.SPUtil;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class PastCtRecAdapter extends RecyclerView.Adapter<PastCtRecAdapter.ViewHolder>{


    public PastCtRecAdapter(Context context, List<PastCtBean.DataBean> list, boolean isCRC, boolean isVoting) {
        this.context = context;
        this.list = list;
        this.isCRC = isCRC;
        this.isVoting = isVoting;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View v;
        if(i==NORMAL_ITEM) {
            v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_ct_past_normal, viewGroup, false);
        } else {
            v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_ct_past_manager, viewGroup, false);
        }
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder viewHolder, int i) {
        PastCtBean.DataBean data = list.get(i);
        viewHolder.time.setText(
                String.format("%1$s — %2$s", DateUtil.formatTimestamp(data.getStartDate(), "yyyy.MM.dd"), DateUtil.formatTimestamp(data.getEndDate(), "yyyy.MM.dd")));
        String status = data.getStatus();
        viewHolder.manager.setVisibility(View.GONE);
        String stage =  data.getIndex() + "";
        int Language = new SPUtil(context).getLanguage();
        if (Language != 0) {
            switch (data.getIndex()) {
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

        if(AppUtlis.isNullOrEmpty(status) || status.equalsIgnoreCase("HISTORY")) {
            //往届
            viewHolder.title.setText(String.format(context.getString(R.string.pastitemtitle), stage, ""));
        } else if(status.equalsIgnoreCase("CURRENT")) {
            //本届
            viewHolder.title.setText(String.format(context.getString(R.string.pastitemtitle), stage,context.getString(R.string.current)));
            if(isCRC) {
                //是委员且质押金大于0
                viewHolder.manager.setText(context.getString(R.string.ctmanager));
                viewHolder.manager.setVisibility(View.VISIBLE);
            }
        } else if(status.equalsIgnoreCase("VOTING")) {
            //选举中
            isVoting = true;
            viewHolder.manager.setVisibility(View.VISIBLE);
            viewHolder.manager.setText(context.getString(R.string.votemanager));
            viewHolder.title.setText(String.format(context.getString(R.string.amember), stage));
        } else {
            viewHolder.title.setText(String.format(context.getString(R.string.pastitemtitle), stage, ""));
        }

        if(isVoting && i==0) {
            viewHolder.manager.setVisibility(View.VISIBLE);
            viewHolder.manager.setText(context.getString(R.string.votemanager));
            viewHolder.title.setText(String.format(context.getString(R.string.pastitemtitle), stage, context.getString(R.string.voting)));
        }

        if(null != managerListener) {
            viewHolder.manager.setOnClickListener(v ->
                    managerListener.onManagerClick(i, isVoting?"VOTING":status)
            );
        }
        if (commonRvListener != null) {
            viewHolder.itemView.setOnClickListener(v -> commonRvListener.onRvItemClick(i, data));
        }
    }

    private static final int NORMAL_ITEM = 0;
    private static final int CARD_ITEM = 1;
    @Override
    public int getItemViewType(int position) {
        String status = list.get(position).getStatus();
        if(!AppUtlis.isNullOrEmpty(status)) {
            if(isCRC && status.equalsIgnoreCase("CURRENT")) {
                return CARD_ITEM;
            }

            if((isVoting&&(0==position)) || status.equalsIgnoreCase("VOTING")) {
                return CARD_ITEM;
            }
        }

        return NORMAL_ITEM;
    }

    @Override
    public int getItemCount() {
        return list==null ? 0 : list.size();
    }

    public interface ManagerListener {
        void onManagerClick(int position, String type);
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.title)
        TextView title;
        @BindView(R.id.time)
        TextView time;
        @BindView(R.id.manager_btn)
        TextView manager;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    public void setManagerListener(ManagerListener listener) {
        this.managerListener = listener;
    }

    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private Context context;
    private ManagerListener managerListener;
    private CommonRvListener commonRvListener;
    private boolean isCRC;
    private boolean isVoting;
    private List<PastCtBean.DataBean> list;
}
