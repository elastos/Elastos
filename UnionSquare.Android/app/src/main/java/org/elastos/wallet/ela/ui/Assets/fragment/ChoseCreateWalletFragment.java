package org.elastos.wallet.ela.ui.Assets.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.ScanQRcodeUtil;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class ChoseCreateWalletFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    Unbinder unbinder;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_chosecreate;
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.create_a_wallet));
    }

    @OnClick({R.id.rl_single, R.id.rl_single_readomly, R.id.rl_mul})
    public void onClicked(View view) {
        switch (view.getId()) {
            case R.id.rl_single:
                start(CreateWalletFragment.class);
                break;
            case R.id.rl_single_readomly:
                requstPermissionOk();
                break;
            case R.id.rl_mul:
                break;
        }
    }

    @Override
    protected void requstPermissionOk() {
        new ScanQRcodeUtil().scanQRcode(this);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        //处理扫描结果（在界面上显示）
        if (resultCode == RESULT_OK && requestCode == ScanQRcodeUtil.SCAN_QR_REQUEST_CODE && data != null) {
            String result = data.getStringExtra("result");//&& matcherUtil.isMatcherAddr(result)
            Bundle bundle = new Bundle();
            bundle.putString("result", result);
            start(CreateSignReadOnlyWalletFragment.class, bundle);
        }

    }
}
