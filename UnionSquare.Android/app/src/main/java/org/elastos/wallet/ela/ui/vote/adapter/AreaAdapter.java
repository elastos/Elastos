package org.elastos.wallet.ela.ui.vote.adapter;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.vote.bean.Area;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;


public class AreaAdapter extends RecyclerView.Adapter<AreaAdapter.ViewHolder> {
    private Context context;
    private List<Area> list;

    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private CommonRvListener commonRvListener;

    public AreaAdapter(Context context, List<Area> list) {
        this.context = context;
        this.list = list;

    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(context).inflate(R.layout.item_area, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        Area area = list.get(position);
      /*  String title = null;
        String currentStr = area.getEn().charAt(0) + "";
        if (position == 0) {
            title = currentStr;
        } else {
            String lastStr = list.get(position - 1).getEn().charAt(0) + "";
            if (!currentStr.equals(lastStr)) {
                title = currentStr;

            }
        }
        if (title == null) {
            holder.line.setVisibility(View.VISIBLE);
            holder.tvTitle.setVisibility(View.GONE);
        } else {
            holder.line.setVisibility(View.GONE);
            holder.tvTitle.setVisibility(View.VISIBLE);
            holder.tvTitle.setText(title);
        }*/
        holder.tvTitle.setVisibility(View.GONE);
        if (position == 0) {
            holder.line.setVisibility(View.GONE);
        } else {
            holder.line.setVisibility(View.VISIBLE);
        }
        // holder.tvCode1.setText("+00" + area.getCode() + "");
        /*int Language = new SPUtil(context).getLanguage();
        String name;
        if (Language == 0) {
            name = area.getZh();
        } else {
            name = area.getEn();
        }*/
        holder.tvName.setText(area.getEn() + " (" + area.getZh() + ")");
        if (commonRvListener != null) {
            holder.itemView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    commonRvListener.onRvItemClick(position, area);
                }
            });
        }
    }


    @Override
    public int getItemCount() {
        return list.size();
    }

    public class ViewHolder extends RecyclerView.ViewHolder {
        @BindView(R.id.tv_title)
        TextView tvTitle;
        @BindView(R.id.rl_view)
        RelativeLayout rlView;
        @BindView(R.id.tv_name1)
        TextView tvName;
        @BindView(R.id.tv_code1)
        TextView tvCode1;
        @BindView(R.id.view_line)
        View line;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }


}
