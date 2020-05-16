package org.elastos.wallet.ela.ui.committee.adaper;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v7.widget.AppCompatImageView;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.committee.bean.SecretaryCtBean;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.svg.GlideApp;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class SecretaryCtRecAdapter extends RecyclerView.Adapter<GeneralCtRecAdapter.ViewHolder> {


    public SecretaryCtRecAdapter(Context context, List<SecretaryCtBean> list) {
        this.context = context;
        this.list = list;
    }


    @NonNull
    @Override
    public GeneralCtRecAdapter.ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_ct_secretary, viewGroup, false);
        return new GeneralCtRecAdapter.ViewHolder(v);
    }

    @Override
    public void onBindViewHolder(@NonNull GeneralCtRecAdapter.ViewHolder viewHolder, int i) {
        SecretaryCtBean data = list.get(i);
        viewHolder.name.setText(data.getName());
        viewHolder.location.setText(data.getLocation());
        GlideApp.with(context).load(data.getAvatar()).error(R.mipmap.icon_ela).into(viewHolder.icon);
        if (commonRvListener != null) {
            viewHolder.itemView.setOnClickListener(v -> commonRvListener.onRvItemClick(i, data));
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
        AppCompatImageView icon;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    private Context context;
    private CommonRvListener commonRvListener;
    private List<SecretaryCtBean> list;
}
