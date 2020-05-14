package org.elastos.wallet.ela.ui.committee.adaper;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.committee.bean.PastCtBean;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.Log;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class PastCtRecAdapter extends RecyclerView.Adapter<PastCtRecAdapter.ViewHolder>{

    public PastCtRecAdapter(Context context, List<PastCtBean> list) {
        this.context = context;
        this.list = list;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View v;
        if(i==0) {
            v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_ct_past_normal, viewGroup, false);
        } else {
            v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_ct_past_manager, viewGroup, false);
        }
        ViewHolder holder = new ViewHolder(v);
        return holder;
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder viewHolder, int i) {
        PastCtBean data = list.get(i);
        viewHolder.title.setText(String.format(context.getString(R.string.ctlisttitle), data.getIndex()));
        viewHolder.time.setText(data.getTime());
        if(data.getType() != 0) {
            viewHolder.manager.setText("委员管理");
            if(null != managerListener) {
                viewHolder.manager.setOnClickListener(v ->
                        managerListener.onManagerClick(i)
                );
            }
        }
        if (commonRvListener != null) {
            viewHolder.itemView.setOnClickListener(v -> commonRvListener.onRvItemClick(i, data));
        }
    }

    @Override
    public int getItemViewType(int position) {
        return list.get(position).getType();
    }

    @Override
    public int getItemCount() {
        return list==null ? 0 : list.size();
    }

    public interface ManagerListener {
        void onManagerClick(int position);
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.title)
        TextView title;
        @BindView(R.id.time)
        TextView time;
        @BindView(R.id.manager_btn)
        TextView manager;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    public void setManagerListener(ManagerListener listener) {
        this.managerListener = listener;
    }

    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private Context context;
    private ManagerListener managerListener;
    private CommonRvListener commonRvListener;
    private List<PastCtBean> list;
}
