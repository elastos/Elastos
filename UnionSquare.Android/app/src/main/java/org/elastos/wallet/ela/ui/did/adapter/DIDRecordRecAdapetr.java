package org.elastos.wallet.ela.ui.did.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.Assets.bean.TransferRecordEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * 资产页面的rv
 */

public class DIDRecordRecAdapetr extends RecyclerView.Adapter<DIDRecordRecAdapetr.ViewHolder> {


    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private CommonRvListener commonRvListener;
    private List<TransferRecordEntity.TransactionsBean> list;

    private Context context;
    private String chainId;

    public DIDRecordRecAdapetr(Context context, List<TransferRecordEntity.TransactionsBean> list, String chainId) {
        this.list = list;
        this.context = context;
        this.chainId = chainId;

    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_did_record, parent, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, final int position) {
        if (commonRvListener != null) {
            holder.itemView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    commonRvListener.onRvItemClick(position, null);
                }
            });
        }
    }

    @Override
    public int getItemCount() {
        return list.size();
    }


    public static class ViewHolder extends RecyclerView.ViewHolder {


        @BindView(R.id.tv_didname)
        TextView tvDidname;
        @BindView(R.id.tv_did)
        TextView tvDid;
        @BindView(R.id.tv_status)
        TextView tvStatus;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }


}
