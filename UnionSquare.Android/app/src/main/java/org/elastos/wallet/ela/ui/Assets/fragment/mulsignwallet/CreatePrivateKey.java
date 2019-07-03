package org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet;

import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.CreateWalletBean;
import org.elastos.wallet.ela.ui.Assets.fragment.BackupPurseFragment;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.widget.keyboard.SecurityEditText;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class CreatePrivateKey extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.et_walletpws)
    SecurityEditText etWalletpws;
    @BindView(R.id.et_walletpws_next)
    SecurityEditText etWalletpwsNext;
    Unbinder unbinder;

    @Override
    protected int getLayoutId() {
        return R.layout.createprivatekey;
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.createprivatekey));
    }

    @OnClick({R.id.tv_create_wallet})
    public void onViewClick(View view) {
        switch (view.getId()) {
            case R.id.tv_create_wallet:
                createwallet();
                break;


        }
    }

    private void createwallet() {

        String pws = etWalletpws.getText().toString().trim();
        String pws_next = etWalletpwsNext.getText().toString().trim();


        if (TextUtils.isEmpty(pws)) {
            showToast(getString(R.string.inputWalletPwd));
            return;
        }
        if (!AppUtlis.chenckString(pws_next)) {
            showToast(getString(R.string.mmgsbd));
            return;
        }
        if (TextUtils.isEmpty(pws_next)) {
            showToast(getString(R.string.inputWalltPwdAgin));
            return;
        }

        if (!pws.equals(pws_next)) {
            showToast(getString(R.string.lcmmsrbyz));
            return;
        }

        CreateWalletBean createWalletBean = new CreateWalletBean();
        createWalletBean.setMasterWalletName("");
        createWalletBean.setPayPassword(pws);
        createWalletBean.setSingleAddress(false);
        Bundle bundle = new Bundle();
        bundle.putParcelable("CreateWalletBean", createWalletBean);
        bundle.putInt("openType", RxEnum.PRIVATEKEY.ordinal());
        start(BackupPurseFragment.class, bundle);
    }


}
