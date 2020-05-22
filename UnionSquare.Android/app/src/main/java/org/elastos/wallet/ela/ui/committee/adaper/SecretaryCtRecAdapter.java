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
import org.elastos.wallet.ela.ui.committee.bean.CtListBean;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.svg.GlideApp;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class SecretaryCtRecAdapter extends RecyclerView.Adapter<SecretaryCtRecAdapter.ViewHolder> {


    public SecretaryCtRecAdapter(Context context, List<CtListBean.Secretariat> list) {
        this.context = context;
        this.list = list;
    }


    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_ct_secretary, viewGroup, false);
        return new ViewHolder(v);
    }

    @Override
    public void onBindViewHolder(ViewHolder viewHolder, int i) {
        CtListBean.Secretariat data = list.get(i);
        viewHolder.name.setText(data.getDidName());
        viewHolder.location.setText(AppUtlis.getLoc(context, String.valueOf(data.getLocation())));
        viewHolder.date.setText(
                String.format("%1$s — %2$s", DateUtil.formatTimestamp(String.valueOf(data.getStartDate()), "yyyy.MM.dd"), DateUtil.formatTimestamp(String.valueOf(data.getEndDate()), "yyyy.MM.dd")));
        String status = data.getStatus();
        if(!AppUtlis.isNullOrEmpty(status) && status.equalsIgnoreCase("CURRENT")) {
            viewHolder.tag.setVisibility(View.VISIBLE);
            viewHolder.tag.setText(context.getString(R.string.incumbent));
        } else {
            viewHolder.tag.setVisibility(View.GONE);
            viewHolder.tag.setText("");
        }
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
        @BindView(R.id.tag)
        TextView tag;
        @BindView(R.id.date)
        TextView date;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    private Context context;
    private CommonRvListener commonRvListener;
    private List<CtListBean.Secretariat> list;
}
