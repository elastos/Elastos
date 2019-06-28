package org.elastos.wallet.ela.ui.Assets.fragment;


import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.CreateWalletBean;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.Log;

import butterknife.BindView;
import butterknife.OnClick;


public class CreateSignReadOnlyWalletFragment extends BaseFragment {
    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.et_walletname)
    ClearEditText etWalletname;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_create_singlereadonly;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        String result = data.getString("result");

    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.createsinglewallet));
    }

    public static CreateWalletFragment newInstance() {
        Bundle args = new Bundle();
        CreateWalletFragment fragment = new CreateWalletFragment();
        fragment.setArguments(args);
        return fragment;
    }

    @OnClick({R.id.sb_create_wallet})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.sb_create_wallet:
                createwallet();
                break;

        }
    }


    private void createwallet() {
        String name = etWalletname.getText().toString().trim();

        if (TextUtils.isEmpty(name)) {
            showToast(getString(R.string.inputWalletName));
            return;
        }

        CreateWalletBean createWalletBean = new CreateWalletBean();
        createWalletBean.setMasterWalletName(name);
        Bundle bundle = new Bundle();
        bundle.putParcelable("CreateWalletBean", createWalletBean);
        start(BackupPurseFragment.class, bundle);
    }

}
