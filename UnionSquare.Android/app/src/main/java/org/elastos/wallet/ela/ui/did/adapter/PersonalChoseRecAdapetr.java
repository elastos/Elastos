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
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.ui.did.entity.PersonalInfoItemEntity;

import java.util.List;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * 资产页面的rv
 */

public class PersonalChoseRecAdapetr extends RecyclerView.Adapter<PersonalChoseRecAdapetr.ViewHolderParent> {


    public void setCommonRvListener(CommonRvListener1 commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    private CommonRvListener1 commonRvListener;


    private Context context;

    private List<PersonalInfoItemEntity> list;

    public PersonalChoseRecAdapetr(Context context,  List<PersonalInfoItemEntity> list) {
        this.context = context;

        this.list = list;


    }

    @Override
    public ViewHolderParent onCreateViewHolder(ViewGroup parent, int viewType) {
        return new ViewHolderParent(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_common_text, parent, false));

    }

    @Override
    public void onBindViewHolder(ViewHolderParent holder, int position) {
        PersonalInfoItemEntity personalInfoItemEntity = list.get(position);
        holder.tv.setText(personalInfoItemEntity.getHintChose());
        if (commonRvListener != null) {
            holder.itemView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    commonRvListener.onRvItemClick(v, position, personalInfoItemEntity);
                }
            });
        }

    }


    @Override
    public int getItemCount() {
        return list.size();
    }

    public static class ViewHolderParent extends RecyclerView.ViewHolder {

        @BindView(R.id.tv)
        TextView tv;

        ViewHolderParent(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }


}
