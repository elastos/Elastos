package org.elastos.wallet.ela.ui.Assets.fragment;


import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.Toast;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.Constant;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

/**
 * 钱包主页
 */
public class HomeWalletFragment extends BaseFragment {


    @BindView(R.id.iv_title_left)
    ImageView ivTitleLeft;
    private String type;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_home_wallet;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void setExtraData(Bundle data) {
        type = data.getString("type");
        if (!Constant.INNER.equals(type)) {
            ivTitleLeft.setVisibility(View.GONE);
        }

        super.setExtraData(data);
    }

    @Override
    protected void initView(View view) {
    }


    @OnClick({R.id.sb_create_wallet, R.id.sb_import_wallet})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.sb_create_wallet:
                start(CreateWalletFragment.newInstance());
                break;
            case R.id.sb_import_wallet:
                start(ImportWalletFragment.newInstance());
                break;
        }
    }

    public static HomeWalletFragment newInstance() {
        Bundle args = new Bundle();
        HomeWalletFragment fragment = new HomeWalletFragment();
        fragment.setArguments(args);
        return fragment;
    }

    private static final long WAIT_TIME = 2000L;
    private long TOUCH_TIME = 0;

    /**
     * 处理回退事件
     *
     * @return
     */
    @Override
    public boolean onBackPressedSupport() {
        if (Constant.INNER.equals(type)) {
            return super.onBackPressedSupport();
        } else {
            if (System.currentTimeMillis() - TOUCH_TIME < WAIT_TIME) {
                _mActivity.finish();
            } else {
                TOUCH_TIME = System.currentTimeMillis();
                Toast.makeText(_mActivity, getString(R.string.press_exit_again), Toast.LENGTH_SHORT).show();
            }
            return true;
        }
    }
}
