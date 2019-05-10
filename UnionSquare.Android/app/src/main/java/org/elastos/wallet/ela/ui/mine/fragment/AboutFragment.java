package org.elastos.wallet.ela.ui.mine.fragment;

import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;

import com.blankj.utilcode.util.ToastUtils;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;

import butterknife.BindView;

/**
 * date: 2018/10/11
 */

public class AboutFragment extends BaseFragment {

    @BindView(R.id.backtitle)
    ImageView backtitle;


    @BindView(R.id.upgradelin)
    LinearLayout mupgradelin;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_about;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {

        backtitle.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                _mActivity.onBackPressed();
            }
        });

        mupgradelin.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ToastUtils.showShort("To do");
            }
        });

    }

    public static AboutFragment newInstance() {
        Bundle args = new Bundle();
        AboutFragment fragment = new AboutFragment();
        fragment.setArguments(args);
        return fragment;
    }

}
