package org.elastos.wallet.ela.ui.committee.fragment;

import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * dismiss prompt(expiration office, impeachment)
 */
public class CtDismissPromptFragment extends BaseFragment {

    String depositAmount;
    String status;
    @BindView(R.id.tv_prompt)
    TextView promptTv;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_dismiss_prompt;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        depositAmount = data.getString("depositAmount");
        status = data.getString("status");
    }

    @Override
    protected void initView(View view) {
        setView();
    }

    private void setView() {

        switch (status) {
            case "Terminated":
                promptTv.setText(getString(R.string.completedialoghint));
                break;
            case "Impeached":
                promptTv.setText(getString(R.string.canceldialoghint));
                break;
            case "Returned":
                promptTv.setText(getString(R.string.dimissdialoghint));
                break;
            default:
                throw new IllegalStateException("Unexpected value: " + status);
        }
    }

    @OnClick({R.id.tv_close, R.id.tv_deposit})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.tv_close:
                popBackFragment();
                break;
            case R.id.tv_deposit:
                Bundle bundle = new Bundle();
                bundle.putString("status", status);
                bundle.putString("depositAmount", depositAmount);
                start(CtManagerFragment.class, bundle);
                break;
        }
    }
}
