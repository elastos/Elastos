package org.elastos.wallet.ela.ui.committee.fragment;

import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;

import butterknife.BindView;

/**
 * secretary general detail fragment
 */
public class SecretaryCtDetailFragment extends BaseFragment {

    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_secretary_detail;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, "秘书长详情");
    }
}
