package org.elastos.wallet.ela.ui.committee.adaper;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.committee.bean.PastCtBean;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;

import java.util.List;

public class PastCtRecAdapter extends RecyclerView.Adapter<PastCtRecAdapter.ViewHolder>{

    public PastCtRecAdapter(Context context, List<PastCtBean> list) {
        this.context = context;
        this.list = list;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_ct_past, viewGroup, false);
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder viewHolder, int i) {
        PastCtBean data = list.get(i);
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

    public static class ViewHolder extends RecyclerView.ViewHolder {

        ViewHolder(View view) {
            super(view);
        }
    }

    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private Context context;
    private CommonRvListener commonRvListener;
    private List<PastCtBean> list;
}
