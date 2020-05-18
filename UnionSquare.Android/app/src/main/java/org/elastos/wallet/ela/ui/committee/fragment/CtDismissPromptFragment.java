package org.elastos.wallet.ela.ui.committee.fragment;

import android.view.View;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;

import butterknife.OnClick;

/**
 * dismiss prompt(expiration office, impeachment)
 */
public class CtDismissPromptFragment extends BaseFragment {
    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_dismiss_prompt;
    }

    @Override
    protected void initView(View view) {

    }

    @OnClick({R.id.tv_close, R.id.tv_deposit})
    public void onClick(View view) {
        switch(view.getId()) {
            case R.id.tv_close:
                popBackFragment();
                break;
            case R.id.tv_deposit:
                //TODO cr manager

                break;
        }
    }
}
