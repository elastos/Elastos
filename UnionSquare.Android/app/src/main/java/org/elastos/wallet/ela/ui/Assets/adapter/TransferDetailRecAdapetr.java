package org.elastos.wallet.ela.ui.Assets.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.Assets.bean.RecorderAddressEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * 交易详情页面的rv
 */
public class TransferDetailRecAdapetr extends RecyclerView.Adapter<TransferDetailRecAdapetr.ViewHolder> {


    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private CommonRvListener commonRvListener;
    private List<RecorderAddressEntity> list;

    private Context context;

    public TransferDetailRecAdapetr(Context context, List<RecorderAddressEntity> list) {
        this.list = list;
        this.context = context;

    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_transfer_detail, parent, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, final int position) {
        RecorderAddressEntity bean = list.get(position + 2);
        holder.tvDetailAddress.setText(bean.getAddress());
        holder.tvDetailAmount.setText(bean.getAmount());
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
        return list.size() > 5 ? 3 :( list.size() - 2);
    }


    public static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.tv_detail_address)
        TextView tvDetailAddress;
        @BindView(R.id.tv_detail_amount)
        TextView tvDetailAmount;
        ViewHolder(View view) {

            super(view);
            ButterKnife.bind(this, view);
        }
    }

}
