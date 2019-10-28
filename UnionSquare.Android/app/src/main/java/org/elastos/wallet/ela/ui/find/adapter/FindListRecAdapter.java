package org.elastos.wallet.ela.ui.find.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.crvote.bean.FindListBean;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * 资产页面的rv
 */

public class FindListRecAdapter extends RecyclerView.Adapter<FindListRecAdapter.ViewHolder> {


    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private CommonRvListener commonRvListener;
    private List<FindListBean> list;

    private Context context;

    public FindListRecAdapter(Context context, List<FindListBean> list) {
        this.list = list;
        this.context = context;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_find_list, parent, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }


    @Override
    public void onBindViewHolder(ViewHolder holder, final int position) {
        FindListBean data = list.get(position);
        holder.imageView.setBackgroundResource(data.getResouceId());
        holder.tvUp.setText(data.getUpText());
        holder.tvDown.setText(data.getDownText());
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

        @BindView(R.id.imageView)
        ImageView imageView;
        @BindView(R.id.tv_up)
        TextView tvUp;
        @BindView(R.id.tv_down)
        TextView tvDown;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

}
