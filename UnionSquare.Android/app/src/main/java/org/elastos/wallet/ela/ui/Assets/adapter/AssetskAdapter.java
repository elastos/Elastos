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
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.LinearInterpolator;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.NumberiUtil;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class AssetskAdapter extends RecyclerView.Adapter<AssetskAdapter.ViewHolder> {
    private final Animation hyperspaceJumpAnimation;
    private Context context;

    public void setData(List<SubWallet> data) {
        this.data = data;
    }

    private List<org.elastos.wallet.ela.db.table.SubWallet> data;
    private CommonRvListener1 commonRvListener;


    public AssetskAdapter(Context context, List<org.elastos.wallet.ela.db.table.SubWallet> data) {
        this.context = context;
        this.data = data;
        hyperspaceJumpAnimation = AnimationUtils.loadAnimation(
                context, R.anim.load_animation);
        hyperspaceJumpAnimation.setInterpolator(new LinearInterpolator());//匀速插值器
    }


    public void setCommonRvListener(CommonRvListener1 commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View view = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_assetsk, viewGroup, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder viewHolder, int i) {
        org.elastos.wallet.ela.db.table.SubWallet assetsItemEntity = data.get(i);
        viewHolder.tvName.setText(assetsItemEntity.getChainId());
        viewHolder.tvNum.setText(NumberiUtil.maxNumberFormat(Arith.div(assetsItemEntity.getBalance(), MyWallet.RATE_S), 12));

        switch (assetsItemEntity.getFiled1()) {
            case "Connecting":
                viewHolder.tvTime.setVisibility(View.GONE);
                viewHolder.tvStatus.setText(context.getString(R.string.connecting));
                viewHolder.tvProgress.setVisibility(View.VISIBLE);
                viewHolder.tvProgress.setText(assetsItemEntity.getProgress() + "%");
                viewHolder.ivSync.setVisibility(View.VISIBLE);
                viewHolder.ivSync.startAnimation(hyperspaceJumpAnimation);
                break;
            case "Disconnected":

                if (assetsItemEntity.getProgress() == 100) {

                    viewHolder.tvTime.setVisibility(View.VISIBLE);
                    viewHolder.tvTime.setText(DateUtil.time(assetsItemEntity.getSyncTime(), context));
                    viewHolder.tvStatus.setText(context.getString(R.string.syncprogress));
                    viewHolder.tvProgress.setVisibility(View.VISIBLE);
                    viewHolder.tvProgress.setText(assetsItemEntity.getProgress() + "%");
                    viewHolder.tvStatus.setText(context.getString(R.string.lastsynctime));
                    viewHolder.ivSync.clearAnimation();
                    viewHolder.ivSync.setVisibility(View.GONE);

                } else {
                    viewHolder.tvTime.setVisibility(View.GONE);
                    viewHolder.tvStatus.setText(context.getString(R.string.connecting));
                    viewHolder.tvProgress.setVisibility(View.VISIBLE);
                    viewHolder.tvProgress.setText(assetsItemEntity.getProgress() + "%");
                    viewHolder.ivSync.setVisibility(View.VISIBLE);
                    viewHolder.ivSync.startAnimation(hyperspaceJumpAnimation);
                }
                break;
            case "Connected":
                viewHolder.tvTime.setVisibility(View.VISIBLE);
                viewHolder.tvTime.setText(DateUtil.time(assetsItemEntity.getSyncTime(), context));
                viewHolder.tvStatus.setText(context.getString(R.string.syncprogress));
                viewHolder.tvProgress.setVisibility(View.VISIBLE);
                viewHolder.tvProgress.setText(assetsItemEntity.getProgress() + "%");
                viewHolder.ivSync.setVisibility(View.VISIBLE);

                viewHolder.ivSync.startAnimation(hyperspaceJumpAnimation);
                if (assetsItemEntity.getProgress() == 100) {
                    viewHolder.tvStatus.setText(context.getString(R.string.lastsynctime));
                    viewHolder.ivSync.clearAnimation();
                    viewHolder.ivSync.setVisibility(View.GONE);
                }
                break;
        }
        if (commonRvListener != null) {
            viewHolder.itemView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    commonRvListener.onRvItemClick(v, i, assetsItemEntity.getChainId());
                }
            });
        }
    }

    @Override
    public int getItemCount() {
        return data.size();
    }


    static class ViewHolder extends RecyclerView.ViewHolder {
        @BindView(R.id.iv_icon)
        ImageView ivIcon;
        @BindView(R.id.tv_name)
        TextView tvName;
        @BindView(R.id.tv_num)
        TextView tvNum;
        @BindView(R.id.tv_status)
        TextView tvStatus;
        @BindView(R.id.tv_time)
        TextView tvTime;
        @BindView(R.id.iv_sync)
        ImageView ivSync;
        @BindView(R.id.tv_progress)
        TextView tvProgress;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }
}
