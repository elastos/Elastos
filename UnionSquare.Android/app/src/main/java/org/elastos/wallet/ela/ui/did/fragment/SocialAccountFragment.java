package org.elastos.wallet.ela.ui.did.fragment;

import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;

import butterknife.BindView;
import butterknife.OnClick;

public class SocialAccountFragment extends BaseFragment {

    @BindView(R.id.tv_title)
    TextView tvTitle;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_socialaccount;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.addsocialaccount));

    }


    @OnClick({R.id.rl_selectsex, R.id.rl_selectbirthday, R.id.rl_selectarea, R.id.tv_title_right, R.id.tv_next})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.rl_selectsex:

                break;
            case R.id.rl_selectbirthday:
                break;


        }
    }


}
