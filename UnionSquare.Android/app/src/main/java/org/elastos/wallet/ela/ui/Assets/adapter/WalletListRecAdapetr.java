package org.elastos.wallet.ela.ui.Assets.adapter;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;


public class WalletListRecAdapetr extends RecyclerView.Adapter<WalletListRecAdapetr.ViewHolder> {


    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private CommonRvListener commonRvListener;
    private List<Wallet> list;

    private Context context;
    private boolean showStatus;

    public WalletListRecAdapetr(Context context, List<Wallet> list) {
        this(context, list, true);

    }

    public WalletListRecAdapetr(Context context, List<Wallet> list, boolean showStatus) {
        this.list = list;
        this.context = context;
        this.showStatus = showStatus;

    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_wallet_list, parent, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, final int position) {
        Wallet data = list.get(position);

        if (!showStatus) {
            holder.view.setVisibility(View.GONE);
        } else if (data.isDefault()) {
            holder.view.setVisibility(View.VISIBLE);
        } else {
            holder.view.setVisibility(View.GONE);
        }
        holder.tvName.setText(data.getWalletName());

        Drawable drawable = context.getDrawable(R.mipmap.single_wallet);
        switch (data.getType()) {
            //0 普通单签 1单签只读 2普通多签 3多签只读
            case 1:
                drawable = context.getDrawable(R.mipmap.single_walllet_readonly);
                break;
            case 2:
                drawable = context.getDrawable(R.mipmap.multi_wallet);
                break;
            case 3:
                drawable = context.getDrawable(R.mipmap.multi_wallet_readonly);
                break;
        }
        drawable.setBounds(0, 0, drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight());
        holder.tvName.setCompoundDrawables(drawable, null, null, null);

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
        @BindView(R.id.view)
        View view;
        @BindView(R.id.tv_name)
        TextView tvName;
        @BindView(R.id.tv_addr)
        TextView tvAddr;
        @BindView(R.id.tv_balance)
        TextView tvBalance;

        ViewHolder(View view) {

            super(view);
            ButterKnife.bind(this, view);
        }
    }

}
