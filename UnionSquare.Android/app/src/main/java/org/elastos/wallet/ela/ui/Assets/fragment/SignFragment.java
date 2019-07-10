package org.elastos.wallet.ela.ui.Assets.fragment;

import android.os.Bundle;
import android.view.View;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;

public class SignFragment extends BaseFragment {
    private String attributes;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_sign;
    }

    @Override
    protected void setExtraData(Bundle data) {
        attributes = data.getString("attributes");
    }

    @Override
    protected void initView(View view) {

    }
}
