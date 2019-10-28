package org.elastos.wallet.ela.ui.Assets.fragment;

import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.presenter.OutportKeystorePresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.OutportKeystoreViewData;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class OutportKeystoreFragment extends BaseFragment implements OutportKeystoreViewData {


    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.et_pwd)
    EditText etPwd;
    @BindView(R.id.et_pwd_agin)
    EditText etPwdAgin;
    Unbinder unbinder;
    @BindView(R.id.tv_1)
    TextView tv1;
    @BindView(R.id.et_payPwd)
    EditText etPayPwd;
    private Wallet wallet;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_outportkeystore;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        wallet = data.getParcelable("wallet");
        if (wallet != null)
            tv1.setText(wallet.getWalletName());
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.outportKeystore);
    }

    @OnClick({R.id.tv_outport})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_outport:
                //确认导出keystore
                outport();
                break;

        }
    }

    private void outport() {
        String payPwd = etPayPwd.getText().toString().trim();
        if (TextUtils.isEmpty(payPwd)) {
            showToastMessage(getString(R.string.walletpwdnotnull));
            return;
        } String pwd = etPwd.getText().toString().trim();
        if (TextUtils.isEmpty(pwd)) {
            showToastMessage(getString(R.string.keystorepwdnotnull));
            return;
        }
        String pwdAgin = etPwdAgin.getText().toString().trim();
        if (TextUtils.isEmpty(pwdAgin)) {
            showToastMessage(getString(R.string.aginpwdnotnull));
            return;
        }
        if (!pwdAgin.equals(pwd)) {
            showToastMessage(getString(R.string.keynotthesame));
            return;
        }

        new OutportKeystorePresenter().exportWalletWithKeystore(wallet.getWalletId(), pwd, payPwd, this);

    }



    @Override
    public void onOutportKeystore(String data) {
        Bundle bundle = new Bundle();
        bundle.putParcelable("wallet", wallet);
        bundle.putString("keyStore", data);
        start(SureOutportKSTFragment.class, bundle);
    }
}
