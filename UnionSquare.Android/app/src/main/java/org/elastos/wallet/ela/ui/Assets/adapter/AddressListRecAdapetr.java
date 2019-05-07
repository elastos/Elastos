package org.elastos.wallet.ela.ui.Assets.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;


public class AddressListRecAdapetr extends RecyclerView.Adapter<AddressListRecAdapetr.ViewHolder> {


    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private CommonRvListener commonRvListener;
    private List<String> list;

    private Context context;

    public AddressListRecAdapetr(Context context, List<String> list) {
        this.list = list;
        this.context = context;

    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_address_list, parent, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, final int position) {
        String data = list.get(position);

        holder.tvAddress.setText(data);
        if (commonRvListener != null) {
            holder.ivCopy.setOnClickListener(new View.OnClickListener() {
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

        @BindView(R.id.tv_address)
        TextView tvAddress;
        @BindView(R.id.iv_copy)
        ImageView ivCopy;
        ViewHolder(View view) {

            super(view);
         ButterKnife.bind(this, view);
        }
    }

}
