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

    public AddMulSignPublicKeyAdapter(CreateMulWalletFragment baseFragment) {
        this.baseFragment = baseFragment;
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
