/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.wallet.ela.ui.Assets.fragment;


import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.Constant;

import butterknife.BindView;
import butterknife.OnClick;

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
        view.setBackgroundResource(R.mipmap.wallet_orignal_bg);
    }


    @OnClick({R.id.sb_create_wallet, R.id.sb_import_wallet})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.sb_create_wallet:
                start(ChoseCreateWalletFragment.class);
                //start(CreateWalletFragment.class);
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
            return closeApp();
        }
    }
}
