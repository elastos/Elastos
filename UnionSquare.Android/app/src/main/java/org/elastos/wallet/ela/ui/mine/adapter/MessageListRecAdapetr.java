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

package org.elastos.wallet.ela.ui.mine.adapter;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.ui.mine.bean.MessageEntity;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.Log;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class MessageListRecAdapetr extends RecyclerView.Adapter<MessageListRecAdapetr.ViewHolder> {
    private List<MessageEntity> list;

    private Context context;

    public MessageListRecAdapetr(Context context, List<MessageEntity> list) {
        this.list = list;
        this.context = context;

    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_message, viewGroup, false);

        return new ViewHolder(v);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder viewHolder, int i) {
        MessageEntity messageEntity = list.get(i);
        viewHolder.tvName.setText(messageEntity.getWalletName());
        String transferType = messageEntity.getTransferType();
        String transferTypeDes = context.getString(R.string.transfertype13);
        String chainId = messageEntity.getChainId();
        try {
            if (MyWallet.IDChain.equals(chainId)) {
                transferTypeDes = context.getString(context.getResources().getIdentifier("sidetransfertype" + transferType, "string",
                        context.getPackageName()));
            } else {
                transferTypeDes = context.getString(context.getResources().getIdentifier("transfertype" + transferType, "string",
                        context.getPackageName()));
            }
        } catch (Exception e) {
            Log.i("transferTypeDes", e.getMessage());
        }
        viewHolder.tvContent.setText("["+transferTypeDes +"]"+ messageEntity.getReason() + ".");
        viewHolder.tvTime.setText(DateUtil.time(messageEntity.getTime(), context));
    }

    @Override
    public int getItemCount() {
        return list.size();
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.tv_name)
        TextView tvName;
        @BindView(R.id.tv_time)
        TextView tvTime;
        @BindView(R.id.tv_content)
        TextView tvContent;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }
}
