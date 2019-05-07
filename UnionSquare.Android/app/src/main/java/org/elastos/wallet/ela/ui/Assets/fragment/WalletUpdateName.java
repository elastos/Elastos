package org.elastos.wallet.ela.ui.Assets.fragment;

import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class WalletUpdateName extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    Unbinder unbinder;
    @BindView(R.id.et_name)
    EditText etName;
    Unbinder unbinder1;
    private String walletId;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_wallet_updatename;
    }

    @Override
    protected void setExtraData(Bundle data) {
        walletId = data.getString("walletId");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.modifywallletname));
    }

    @OnClick({R.id.tv_modify})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_modify:
                String walletName = etName.getText().toString().trim();
                if (TextUtils.isEmpty(walletName)) {
                    showToastMessage(getString(R.string.walletnamenotnull));
                    return;
                }

                new RealmUtil().upDataWalletName(walletId, walletName);
                post(RxEnum.UPDATA_WALLET_NAME.ordinal(), walletName, walletId);
                showToastMessage(getString(R.string.modifysucess));
                popBackFragment();
                break;

        }
    }


}
