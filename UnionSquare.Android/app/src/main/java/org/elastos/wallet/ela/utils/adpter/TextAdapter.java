package org.elastos.wallet.ela.utils.adpter;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;


public class TextAdapter extends RecyclerView.Adapter<TextAdapter.MyViewHolder> {


    private List<String> temp;
    private Context context;


    public TextAdapter(List<String> temp, Context context) {
        this.temp = temp;
        this.context = context;

    }


    @NonNull
    @Override
    public MyViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(context).inflate(R.layout.item_text, parent, false);

        return new MyViewHolder(v);
    }


    @Override
    public void onBindViewHolder(@NonNull MyViewHolder holder, int position) {

        String text = temp.get(position);
        holder.tvIp.setText(text);


        (holder.ivDel).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (temp.size() > 0) {
                    temp.remove(position);
                    notifyItemRemoved(position);//加动画
                    //notifyDataSetChanged();//会没动画
                    if (position != temp.size())
                        notifyItemRangeChanged(position, temp.size() - position);

                }
            }
        });

        if (null != onItemOnclickListner) {
            (holder.itemView).setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    onItemOnclickListner.onItemClick(v, position, text);

                }
            });
        }

    }

    public void setOnItemOnclickListner(OnItemClickListner onItemOnclickListner) {
        this.onItemOnclickListner = onItemOnclickListner;

    }

    private OnItemClickListner onItemOnclickListner;

    public interface OnItemClickListner {
        void onItemClick(View v, int position, String text);
    }

    @Override
    public int getItemCount() {
        return temp.size();
    }


    static class MyViewHolder extends RecyclerView.ViewHolder {
        @BindView(R.id.iv_del)
        ImageView ivDel;
        @BindView(R.id.tv_ip)
        TextView tvIp;

        MyViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

}