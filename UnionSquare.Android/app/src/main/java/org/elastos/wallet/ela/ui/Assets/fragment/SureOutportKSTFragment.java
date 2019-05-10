package org.elastos.wallet.ela.ui.Assets.fragment;

import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.utils.ClipboardUtil;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class SureOutportKSTFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.et_1)
    EditText et1;
    Unbinder unbinder;
    @BindView(R.id.tv_1)
    TextView tv1;
    private Wallet wallet;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_sureoutportkst;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        wallet = data.getParcelable("wallet");
        if (wallet != null) {
            tv1.setText(wallet.getWalletName());
            et1.setText(wallet.getKeyStore());
        }
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.outportKeystore));
    }

    @OnClick({R.id.tv_copy})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_copy:
                new ClipboardUtil().copyClipboar(getBaseActivity(), et1.getText().toString().trim());
                break;

        }
    }


}
