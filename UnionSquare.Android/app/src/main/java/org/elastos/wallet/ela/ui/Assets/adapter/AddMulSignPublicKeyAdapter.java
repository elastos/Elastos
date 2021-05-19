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

import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.text.Editable;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet.CreateMulWalletFragment;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.ClipboardUtil;

import java.util.HashMap;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;

public class AddMulSignPublicKeyAdapter extends RecyclerView.Adapter<AddMulSignPublicKeyAdapter.ViewHolder> {
    private int count;

    public Map<Integer, String> getMap() {
        return map;
    }

    private Map<Integer, String> map;
    private CreateMulWalletFragment baseFragment;

    public void setCount(int count) {
        this.count = count;

    }

    private String defaultKey;

    public AddMulSignPublicKeyAdapter(CreateMulWalletFragment baseFragment, String defaultKey) {
        this.baseFragment = baseFragment;
        this.defaultKey = defaultKey;
        if (map == null) {
            map = new HashMap<>();
        } else {
            map.clear();
        }
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_add_publickey, viewGroup, false);
        return new ViewHolder(v);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder viewHolder, int i) {
        if (!TextUtils.isEmpty(defaultKey) && i == 0 && TextUtils.isEmpty(viewHolder.etPublickey.getText().toString().trim())) {
            viewHolder.etPublickey.setText(defaultKey);
        }
        viewHolder.ivPaste.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                viewHolder.etPublickey.setText(ClipboardUtil.paste(baseFragment.getContext()));
            }
        });
        viewHolder.ivScan.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                baseFragment.requstManifestPermission(baseFragment.getString(R.string.needpermission), viewHolder.etPublickey);
            }
        });
        /*if (!TextUtils.isEmpty(viewHolder.etPublickey.getText().toString().trim())) {
            map.put(i, viewHolder.etPublickey.getText().toString().trim());
        }*/
        viewHolder.etPublickey.addTextChangedListener(new ClearEditText(baseFragment.getContext()) {
            @Override
            public void afterTextChanged(Editable s) {
                if (!TextUtils.isEmpty(s.toString())) {
                    map.put(i, s.toString().trim());
                } else {
                    map.remove(i);
                }
            }
        });
    }

    @Override
    public int getItemCount() {
        return count;
    }


    private OnViewClickListener onViewClickListener;

    public void setOnViewClickListener(OnViewClickListener onViewClickListener) {
        this.onViewClickListener = onViewClickListener;
    }

    public interface OnViewClickListener {
        void onItemViewClick(View clickView, View targetView);
    }

    static
    class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.et_publickey)
        ClearEditText etPublickey;
        @BindView(R.id.iv_scan)
        ImageView ivScan;
        @BindView(R.id.iv_paste)
        ImageView ivPaste;

        ViewHolder(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }


}
