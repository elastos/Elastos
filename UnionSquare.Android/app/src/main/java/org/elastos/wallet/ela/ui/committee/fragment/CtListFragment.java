package org.elastos.wallet.ela.ui.committee.fragment;

import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;

import butterknife.BindView;

/**
 * Committee and secretary general list container
 */
public class CtListFragment extends BaseFragment {
    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_list;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.crcvote), getString(R.string.voting_rules));

    }
}
