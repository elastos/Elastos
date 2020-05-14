package org.elastos.wallet.ela.ui.committee.fragment;

import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * committee manage fragment(in office, expiration office, impeachment)
 */
public class CtManagerFragment extends BaseFragment {
    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_manager;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, "委员管理");
    }


    @OnClick({R.id.refresh_ct_info, R.id.refresh_ct_did})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.refresh_ct_info:
                start(CtManagerEditFragment.class);
                break;
            case R.id.refresh_ct_did:
                Toast.makeText(getContext(), "选举授权页", Toast.LENGTH_SHORT).show();
                break;
        }
    }

}
