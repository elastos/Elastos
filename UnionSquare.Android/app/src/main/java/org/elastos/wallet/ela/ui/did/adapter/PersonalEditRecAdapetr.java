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
import android.text.InputFilter;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.ui.did.entity.PersonalInfoItemEntity;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * 资产页面的rv
 */

public class PersonalEditRecAdapetr extends RecyclerView.Adapter<PersonalEditRecAdapetr.ViewHolderParent> {


    public void setCommonRvListener(CommonRvListener1 commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private CommonRvListener1 commonRvListener;


    private Context context;

    private List<PersonalInfoItemEntity> list;

    public PersonalEditRecAdapetr(Context context, List<PersonalInfoItemEntity> list) {
        this.context = context;

        this.list = list;


    }

    @Override
    public ViewHolderParent onCreateViewHolder(ViewGroup parent, int viewType) {
        if (viewType == 2)
            //两个输入框+两个textview 电话
            return new ViewHolder2(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_personalinfo_2input_2tv, parent, false));
        else if (viewType == 1)
            //个人简介 生日 出生
            return new ViewHolder1(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_personalinfo_2textviewt, parent, false));
        else
            //通用的 tv+edit
            return new ViewHolder0(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_personalinfo_input_tv, parent, false));

    }

    @Override
    public void onBindViewHolder(ViewHolderParent holder, int position) {
        //  holder.setIsRecyclable(false);
        PersonalInfoItemEntity personalInfoItemEntity = list.get(position);
        int index = personalInfoItemEntity.getIndex();
        if (holder instanceof ViewHolder2) {
            ((ViewHolder2) holder).tv1.setText(personalInfoItemEntity.getHintShow1());
            ((ViewHolder2) holder).tv2.setText(personalInfoItemEntity.getHintShow2());
            ((ViewHolder2) holder).et1.setText(personalInfoItemEntity.getText1());
            ((ViewHolder2) holder).et2.setText(personalInfoItemEntity.getText2());

        } else if (holder instanceof ViewHolder1) {
            ((ViewHolder1) holder).tv1.setText(personalInfoItemEntity.getHintShow1());
            ((ViewHolder1) holder).tv2.setText(personalInfoItemEntity.getText1());
            ((ViewHolder1) holder).itemView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (commonRvListener != null)
                        commonRvListener.onRvItemClick(((ViewHolder1) holder).tv2, position, personalInfoItemEntity);
                }
            });

        } else {

            ((ViewHolder0) holder).tv1.setHint(personalInfoItemEntity.getHintShow1());
            if (index == 0) {
                ((ViewHolder0) holder).et1.setFilters(new InputFilter[]{new InputFilter.LengthFilter(16)});
            } else if (index < 9) {
                ((ViewHolder0) holder).et1.setFilters(new InputFilter[]{new InputFilter.LengthFilter(100)});
            } else {
                ((ViewHolder0) holder).et1.setFilters(new InputFilter[]{new InputFilter.LengthFilter(50)});
            }
            ((ViewHolder0) holder).et1.setText(personalInfoItemEntity.getText1());
        }
        if (commonRvListener != null) {
            holder.iv.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    commonRvListener.onRvItemClick(v, position, personalInfoItemEntity);
                }
            });
        }

    }

    @Override
    public int getItemViewType(int position) {
        int index = list.get(position).getIndex();
        if (index == 1 || index == 2 || index == 6 || index == 7) {
            return 1;
        }
        if (index == 5) {
            //手机号
            return 2;
        }
        return 0;
    }

    @Override
    public int getItemCount() {
        return list.size();
    }

    public static class ViewHolderParent extends RecyclerView.ViewHolder {

        @BindView(R.id.iv)
        ImageView iv;

        ViewHolderParent(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    public static class ViewHolder0 extends ViewHolderParent {
        @BindView(R.id.et1)
        EditText et1;
        @BindView(R.id.tv1)
        TextView tv1;

        ViewHolder0(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    class ViewHolder1 extends ViewHolderParent {


        @BindView(R.id.tv1)
        TextView tv1;
        @BindView(R.id.tv2)
        TextView tv2;

        ViewHolder1(View view) {

            super(view);
            tv1.setTextColor(context.getResources().getColor(R.color.whiter50));
            ButterKnife.bind(this, view);
        }
    }

    class ViewHolder2 extends ViewHolderParent {
        @BindView(R.id.et1)
        EditText et1;
        @BindView(R.id.et2)
        EditText et2;
        @BindView(R.id.tv1)
        TextView tv1;
        @BindView(R.id.tv2)
        TextView tv2;

        ViewHolder2(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }


}
