package org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet;

import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.ImportKeystoreFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.WalletListFragment;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class MainPrivateKeyFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_mainprivatekey;
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.addmainkey));
    }

    @OnClick({R.id.rl_new, R.id.rl_import, R.id.rl_use})
    public void onViewClick(View view) {
        switch (view.getId()) {
            case R.id.rl_new:
                start(CreatePrivateKey.class);
                break;
            case R.id.rl_import:
                start(ImportMnemonicPageFragment.class);
                break;
            case R.id.rl_use:
                Bundle bundle=new Bundle();
                bundle.putString("openType","showList");
                start(WalletListFragment.class,bundle);
                break;

        }
    }


}
