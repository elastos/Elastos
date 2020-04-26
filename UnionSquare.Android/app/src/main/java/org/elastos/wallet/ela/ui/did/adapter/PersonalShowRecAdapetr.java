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

package org.elastos.wallet.ela.ui.did.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.ui.did.entity.PersonalInfoItemEntity;
import org.elastos.wallet.ela.utils.svg.GlideApp;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * 资产页面的rv
 */

public class PersonalShowRecAdapetr extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

    private Context context;

    public void setCommonRvListener(CommonRvListener1 commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private CommonRvListener1 commonRvListener;
    private List<PersonalInfoItemEntity> list;

    public PersonalShowRecAdapetr(Context context, List<PersonalInfoItemEntity> list) {
        this.context = context;

        this.list = list;


    }

    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        if (viewType == 1)
            //展示照片
            return new ViewHolder1(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_personalinfo_tviv, parent, false));
        else
            return new ViewHolder0(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_personalinfo_2textviewt_show, parent, false));

    }

    @Override
    public void onBindViewHolder(RecyclerView.ViewHolder holder, int position) {
        //  holder.setIsRecyclable(false);
        PersonalInfoItemEntity personalInfoItemEntity = list.get(position);
        int index = personalInfoItemEntity.getIndex();
        if (holder instanceof ViewHolder1) {
            //头像
            ((ViewHolder1) holder).tv1.setText(personalInfoItemEntity.getHintShow1());
            String logo = personalInfoItemEntity.getText1();
            GlideApp.with(context).load(logo).error(R.mipmap.mine_did_default_avator)
                    .into(((ViewHolder1) holder).iv1);
        } else {
            ((ViewHolder0) holder).tv1.setText(personalInfoItemEntity.getHintShow1());
            ((ViewHolder0) holder).tv2.setText(personalInfoItemEntity.getText1());
            if (index == 5) {
                //电话
                ((ViewHolder0) holder).tv1.setText(personalInfoItemEntity.getHintShow2());
                String result = personalInfoItemEntity.getText1() + " " + personalInfoItemEntity.getText2();
                if (personalInfoItemEntity.getText1() == null) {
                    result = personalInfoItemEntity.getText2();
                } else if (personalInfoItemEntity.getText2() == null) {
                    result = personalInfoItemEntity.getText1();
                }
                ((ViewHolder0) holder).tv2.setText(result);
            }
            if (index == 7) {
                ((ViewHolder0) holder).itemView.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if (commonRvListener != null)
                            commonRvListener.onRvItemClick(((ViewHolder0) holder).tv2, position, personalInfoItemEntity);
                    }
                });
            }
        }


    }

    @Override
    public int getItemViewType(int position) {
        int index = list.get(position).getIndex();
        if (index == 3) {
            //头像
            return 1;
        }
        return 0;
    }

    @Override
    public int getItemCount() {
        return list.size();
    }


    public static class ViewHolder0 extends RecyclerView.ViewHolder {


        @BindView(R.id.tv1)
        TextView tv1;
        @BindView(R.id.tv2)
        TextView tv2;

        ViewHolder0(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    public static class ViewHolder1 extends RecyclerView.ViewHolder {

        @BindView(R.id.tv1)
        TextView tv1;
        @BindView(R.id.iv1)
        ImageView iv1;

        ViewHolder1(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }


}
