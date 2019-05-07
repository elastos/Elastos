package org.elastos.wallet.ela.ui.Assets.adapter;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v7.widget.AppCompatSeekBar;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.Assets.bean.AssetsItemEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.NumberiUtil;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class AssetskAdapter extends RecyclerView.Adapter<AssetskAdapter.ViewHolder> {
    private Context context;
    private List<AssetsItemEntity> data;
    private CommonRvListener1 commonRvListener;


    public AssetskAdapter(Context context, List<AssetsItemEntity> data) {
        this.context = context;
        this.data = data;
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
        AssetsItemEntity assetsItemEntity = data.get(i);
        viewHolder.tvName.setText(assetsItemEntity.getChainId());
        viewHolder.tvTime.setText(context.getString(R.string.lastsynctime) + assetsItemEntity.getSyncRime());
        viewHolder.sbSuger.setEnabled(false);
        viewHolder.sbSuger.setProgress(assetsItemEntity.getProgress());
        viewHolder.tvProgress.setText(assetsItemEntity.getProgress() + "%");
       // viewHolder.tvNum.setText(NumberiUtil.maxNumberFormat((Double.parseDouble(assetsItemEntity.getBalance()) / MyWallet.RATE) + "", 12));
        viewHolder.tvNum.setText(NumberiUtil.maxNumberFormat(Arith.div(assetsItemEntity.getBalance(),MyWallet.RATE_S), 12));
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
        @BindView(R.id.tv_progress)
        TextView tvProgress;
        @BindView(R.id.tv_num)
        TextView tvNum;
        @BindView(R.id.sb_suger)
        AppCompatSeekBar sbSuger;
        @BindView(R.id.tv_time)
        TextView tvTime;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }
}
