/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
import org.elastos.wallet.ela.ui.Assets.bean.IPEntity;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;


public class TextAdapter extends RecyclerView.Adapter<TextAdapter.MyViewHolder> {


    private List<IPEntity> temp;
    private Context context;


    public TextAdapter(List<IPEntity> temp, Context context) {
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

        IPEntity entity = temp.get(position);
        holder.tvIp.setText(entity.getAddress());
        holder.tvPort.setText(entity.getPort() + "");


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
                    onItemOnclickListner.onItemClick(v, position, entity);

                }
            });
        }

    }

    public void setOnItemOnclickListner(OnItemClickListner onItemOnclickListner) {
        this.onItemOnclickListner = onItemOnclickListner;

    }

    private OnItemClickListner onItemOnclickListner;

    public interface OnItemClickListner {
        void onItemClick(View v, int position, IPEntity ipEntity);
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
        @BindView(R.id.tv_port)
        TextView tvPort;

        MyViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

}
