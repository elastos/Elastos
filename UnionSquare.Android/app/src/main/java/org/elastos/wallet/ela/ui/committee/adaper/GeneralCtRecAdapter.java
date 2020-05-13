package org.elastos.wallet.ela.ui.committee.adaper;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.committee.bean.GeneralCtBean;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.svg.GlideApp;
import org.elastos.wallet.ela.utils.widget.RoundImageView;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class GeneralCtRecAdapter extends RecyclerView.Adapter<GeneralCtRecAdapter.ViewHolder> {

    public GeneralCtRecAdapter(Context context, List<GeneralCtBean> list) {
        this.context = context;
        this.list = list;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_ct_general, viewGroup, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder viewHolder, int i) {
        GeneralCtBean data = list.get(i);
        viewHolder.name.setText(data.getName());
        viewHolder.location.setText(data.getLocation());
        GlideApp.with(context).load(data.getUrl()).error(R.mipmap.icon_ela).into(viewHolder.icon);
        if (commonRvListener != null) {
            viewHolder.itemView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    commonRvListener.onRvItemClick(i, data);
                }
            });
        }
    }

    @Override
    public int getItemCount() {
        return list==null ? 0 : list.size();
    }

    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.name)
        TextView name;
        @BindView(R.id.location)
        TextView location;
        @BindView(R.id.icon)
        RoundImageView icon;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    private Context context;
    private CommonRvListener commonRvListener;
    private List<GeneralCtBean> list;
}
