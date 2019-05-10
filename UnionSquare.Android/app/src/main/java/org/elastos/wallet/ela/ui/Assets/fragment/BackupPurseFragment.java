package org.elastos.wallet.ela.ui.Assets.fragment;


import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 备份钱包
 */
public class BackupPurseFragment extends BaseFragment {


    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_backup_purse;
    }

    @Override
    protected void initInjector() {

    }

    Bundle bundle;

    @Override
    protected void setExtraData(Bundle data) {
        bundle = data;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.backup_the_purse));
    }


    @OnClick({R.id.toolbar, R.id.sb_backup_the_purse})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.toolbar:
                break;
            case R.id.sb_backup_the_purse:
                start(MnemonicWordFragment.class, bundle);
                break;
        }
    }

    public static BackupPurseFragment newInstance() {
        Bundle args = new Bundle();
        BackupPurseFragment fragment = new BackupPurseFragment();
        fragment.setArguments(args);
        return fragment;
    }
}
