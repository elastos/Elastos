package org.elastos.wallet.ela.ui.committee.fragment;

import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.view.CircleProgressView;

import butterknife.BindView;

/**
 * general committee detail
 */
public class GeneralCtDetailFragment extends BaseFragment {
    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.progress)
    CircleProgressView progress;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_general_detail;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, "委员详情");
        progress.setProgress(50);
    }


}
