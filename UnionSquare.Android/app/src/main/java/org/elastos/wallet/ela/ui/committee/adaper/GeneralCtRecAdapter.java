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

package org.elastos.wallet.ela.ui.committee.adaper;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v7.widget.AppCompatImageView;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.committee.bean.CtListBean;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.svg.GlideApp;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class GeneralCtRecAdapter extends RecyclerView.Adapter<GeneralCtRecAdapter.ViewHolder> {

    public GeneralCtRecAdapter(Context context, List<CtListBean.Council> list) {
        this.context = context;
        this.list = list;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_ct_general, viewGroup, false);
        return new ViewHolder(v);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder viewHolder, int i) {
        CtListBean.Council data = list.get(i);
        viewHolder.name.setText(data.getDidName());
        viewHolder.location.setText(AppUtlis.getLoc(context, String.valueOf(data.getLocation())));
        String status = data.getStatus();
        viewHolder.tag.setVisibility(View.GONE);
        viewHolder.tag.setText("");
        if(!AppUtlis.isNullOrEmpty(status)) {
            if(status.equalsIgnoreCase("Returned")
            || status.equalsIgnoreCase("Impeached")) {
                viewHolder.tag.setVisibility(View.VISIBLE);
                viewHolder.tag.setText(context.getString(R.string.disbanded));
            }
        }
        GlideApp.with(context).load(data.getAvatar()).error(R.mipmap.icon_ela).circleCrop().into(viewHolder.icon);
        if (commonRvListener != null) {
            viewHolder.itemView.setOnClickListener(v -> commonRvListener.onRvItemClick(i, data));
        }
    }

    @Override
    public int getItemCount() {
        return list==null ? 0 : list.size();
    }

    public void setCommonRvListener(CommonRvListener commonRvListener) {
        this.commonRvListener = commonRvListener;
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.name)
        TextView name;
        @BindView(R.id.location)
        TextView location;
        @BindView(R.id.icon)
        AppCompatImageView icon;
        @BindView(R.id.tag)
        TextView tag;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }

    private Context context;
    private CommonRvListener commonRvListener;
    private List<CtListBean.Council> list;
}
