package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;

import butterknife.BindView;

public class ShowPersonalIntroFragemnt extends BaseFragment {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_intro)
    TextView tvIntro;


    String personalIntro;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_personalintro_show;
    }

    @Override
    protected void setExtraData(Bundle data) {
        personalIntro = data.getString("personalIntro");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.personalintro));
        if (!TextUtils.isEmpty(personalIntro)) {
            tvIntro.setText(personalIntro);
        }
    }


}