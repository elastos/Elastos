package org.elastos.wallet.ela.ui.committee.adaper;

import android.content.Context;
import android.graphics.Color;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.committee.bean.ExperienceBean;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * Performance record
 */
public class CtExpRecAdapter extends RecyclerView.Adapter<CtExpRecAdapter.ViewHolder> {

    public CtExpRecAdapter(Context context, List<ExperienceBean> list) {
        this.context = context;
        this.list = list;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_ct_detail_experience, viewGroup, false);
        return new ViewHolder(v);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder viewHolder, int i) {
        ExperienceBean data = list.get(i);
        viewHolder.title.setText(data.getTitle());
        viewHolder.subTitle.setText(data.getSubTitle());
        if(data.getType() == 0) {
            viewHolder.tag.setBackgroundColor(Color.parseColor("#666666"));
            viewHolder.tag.setText("弃权");
        } else if(data.getType() == 1) {
            viewHolder.tag.setBackgroundColor(Color.parseColor("#B04135"));
            viewHolder.tag.setText("反对");
        } else if(data.getType() == 2) {
            viewHolder.tag.setBackgroundColor(Color.parseColor("#35B08F"));
            viewHolder.tag.setText("赞成");
        } else {
            viewHolder.tag.setBackgroundColor(Color.parseColor("#00000000"));
        }

        if (commonRvListener != null) {
            viewHolder.itemView.setOnClickListener(v -> commonRvListener.onRvItemClick(i, data));
        }
    }

    @Override
    public int getItemCount() {
        return list==null ? 0: list.size();
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        @BindView(R.id.title)
        TextView title;
        @BindView(R.id.sub_title)
        TextView subTitle;
        @BindView(R.id.tag)
        TextView tag;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private Context context;
    private CommonRvListener commonRvListener;
    private List<ExperienceBean> list;
}
