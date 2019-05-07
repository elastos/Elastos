package org.elastos.wallet.ela.ui.Assets.fragment;

import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.presenter.WalletUpdatePwdPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.UpdataWalletPwdViewData;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class WalletUpdataPwdFragment extends BaseFragment implements UpdataWalletPwdViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    Unbinder unbinder;
    @BindView(R.id.et_pwd_origin)
    EditText etPwdOrigin;
    @BindView(R.id.et_pwd_new)
    EditText etPwdNew;
    @BindView(R.id.et_pwd_agin)
    EditText etPwdAgin;
    Unbinder unbinder1;
    private String walletId;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_wallet_updatepwd;
    }

    @Override
    protected void setExtraData(Bundle data) {
        walletId = data.getString("walletId");
    }


    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.modifywallletpwd));
    }

    @OnClick({R.id.tv_modify})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_modify:
                String originPwd = etPwdOrigin.getText().toString().trim();
                if (TextUtils.isEmpty(originPwd)) {
                    showToastMessage(getString(R.string.originpwdnotnull));
                    return;
                }
                String newPwd = etPwdNew.getText().toString().trim();
                if (TextUtils.isEmpty(originPwd)) {
                    showToastMessage(getString(R.string.newpwdnotnull));
                    return;
                }
                String aginPwd = etPwdAgin.getText().toString().trim();
                if (TextUtils.isEmpty(aginPwd)) {
                    showToastMessage(getString(R.string.aginpwdnotnull));
                    return;
                }
                if (!newPwd.equals(aginPwd)) {
                    showToastMessage(getString(R.string.keynotthesame));
                    return;
                }
                new WalletUpdatePwdPresenter().changePassword(walletId, originPwd, newPwd, this);

                break;

        }
    }


    @Override
    public void onUpdataPwd(String data) {
        showToastMessage(getString(R.string.modifysucess));
        popBackFragment();
    }
}