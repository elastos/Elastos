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

package org.elastos.wallet.ela.ui.Assets.adapter;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.Assets.bean.Word;
import org.elastos.wallet.ela.utils.ScreenUtil;

import java.util.List;
import java.util.Random;


public class CommonTextViewAdapter extends RecyclerView.Adapter<CommonTextViewAdapter.MyViewHolder> {

    private List<Word> temp;
    private Context context;

    public CommonTextViewAdapter(List<Word> temp, Context context) {
        this.temp = temp;
        this.context = context;
    }


    @NonNull
    @Override
    public MyViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        TextView textView = new TextView(context);
        textView.setTextColor(context.getResources().getColor(R.color.qmui_config_color_white));
        //  textView.setFocusable(true);
        //     textView.setFocusableInTouchMode(true);
        textView.setTextSize(15);
        textView.setGravity(Gravity.CENTER);
        textView.setPadding(0, ScreenUtil.dp2px(context,11), 0,  ScreenUtil.dp2px(context,11));
        MyViewHolder holder = new MyViewHolder(textView);
        return holder;
    }

    public void moveRandomData() {
        for (int i = 0; i < temp.size(); i++) {
            notifyItemMoved(i, new Random().nextInt(i + 1));
        }
    }

    /**
     * notifyItemRangeChanged会开启多个线程
     *
     * @param position
     */
    public void removeData(int position) {
        if (position == temp.size()) {
            //防止越界
            return;
        }
        temp.remove(position);
        notifyItemRemoved(position);//加动画
        //notifyDataSetChanged();//会没动画
        notifyItemRangeChanged(position, temp.size() - position);
    }

    @Override
    public void onBindViewHolder(@NonNull MyViewHolder holder, final int position) {

        ((TextView) (holder.itemView)).setText(temp.get(position).getWord());
        if (null != onItemOnclickListner) {
            (holder.itemView).setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    onItemOnclickListner.onItemClick(v, position);

                }
            });
        }
    }


    @Override
    public int getItemCount() {
        return temp.size();
    }

    class MyViewHolder extends RecyclerView.ViewHolder {
        MyViewHolder(TextView itemView) {
            super(itemView);
        }
    }

    public void setOnItemOnclickListner(OnItemClickListner onItemOnclickListner) {
        this.onItemOnclickListner = onItemOnclickListner;
    }

    private OnItemClickListner onItemOnclickListner;

    public interface OnItemClickListner {
        void onItemClick(View v, int position);
    }
}
