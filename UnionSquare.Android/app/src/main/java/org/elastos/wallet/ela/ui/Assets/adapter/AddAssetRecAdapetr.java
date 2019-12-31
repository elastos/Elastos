package org.elastos.wallet.ela.ui.Assets.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;

import java.util.ArrayList;

import butterknife.BindView;
import butterknife.ButterKnife;


/**
 * Created by wangdongfeng on 2018/4/14.
 */

public class AddAssetRecAdapetr extends RecyclerView.Adapter<AddAssetRecAdapetr.ViewHolder> {

    private CommonRvListener1 commonRvListener;
    private String[] datas;

    private Context context;
    ArrayList<String> contains;

    public AddAssetRecAdapetr(Context context, String[] datas, ArrayList<String> contains) {
        this.datas = datas;
        this.context = context;
        this.contains = contains;
    }

    public void setCommonRvListener(CommonRvListener1 commonRvListener) {
        this.commonRvListener = commonRvListener;
    }


    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_asset_add, parent, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, final int position) {
        String data = datas[position];
        if (position == 0) {
            holder.ivStatus.setVisibility(View.GONE);
        } else {
            holder.ivStatus.setVisibility(View.VISIBLE);
        }
        if (contains.contains(data)) {
            holder.ivStatus.setSelected(true);
        } else {
            holder.ivStatus.setSelected(false);
        }
        holder.tvName.setText(data);
        if (commonRvListener != null) {
            holder.ivStatus.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    commonRvListener.onRvItemClick(v, position, data);
                }
            });
        }
    }

    @Override
    public int getItemCount() {
        return datas.length;
    }


    public static class ViewHolder extends RecyclerView.ViewHolder {
        @BindView(R.id.tv_name)
        TextView tvName;
        @BindView(R.id.iv_status)
        ImageView ivStatus;

        ViewHolder(View view) {

            super(view);
            ButterKnife.bind(this, view);
        }
    }

}
