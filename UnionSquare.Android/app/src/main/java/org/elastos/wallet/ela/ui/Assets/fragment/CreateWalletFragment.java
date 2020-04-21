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
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.View;
import android.widget.CheckBox;
import android.widget.TextView;

import com.qmuiteam.qmui.layout.QMUILinearLayout;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.CreateWalletBean;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.widget.keyboard.SecurityEditText;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 创建钱包
 */
public class CreateWalletFragment extends BaseFragment {
    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.et_walletname)
    ClearEditText etWalletname;
    @BindView(R.id.et_walletpws)
    SecurityEditText etWalletpws;
    @BindView(R.id.et_walletpws_next)
    SecurityEditText etWalletpwsNext;
    @BindView(R.id.creat_wallet_red_chechbox)
    CheckBox creatWalletRedChechbox;
    @BindView(R.id.ll_create_wallet)
    QMUILinearLayout ll_create_wallet;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_create_wallet;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.create_a_wallet));
        //开启自定义键盘
        // AppUtlis.securityKeyboard(ll_create_wallet);
    }

    public static CreateWalletFragment newInstance() {
        Bundle args = new Bundle();
        CreateWalletFragment fragment = new CreateWalletFragment();
        fragment.setArguments(args);
        return fragment;
    }

    @OnClick({R.id.sb_create_wallet, R.id.sb_import_wallet})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.sb_create_wallet:
                createwallet();
                break;
            case R.id.sb_import_wallet:
                break;
        }
    }


    private void createwallet() {
        String name = etWalletname.getText().toString().trim();
        String pws = etWalletpws.getText().toString().trim();
        String pws_next = etWalletpwsNext.getText().toString().trim();
        if (TextUtils.isEmpty(name)) {
            showToast(getString(R.string.inputWalletName));
            return;
        }


        if (TextUtils.isEmpty(pws)) {
            showToast(getString(R.string.inputWalletPwd));
            return;
        }
        if (!AppUtlis.chenckString(pws_next)) {
            showToast(getString(R.string.mmgsbd));
            return;
        }
        if (TextUtils.isEmpty(pws_next)) {
            showToast(getString(R.string.inputWalltPwdAgin));
            return;
        }

        if (!pws.equals(pws_next)) {
            showToast(getString(R.string.lcmmsrbyz));
            return;
        }
        CreateWalletBean createWalletBean = new CreateWalletBean();
        createWalletBean.setMasterWalletName(name);
        createWalletBean.setPayPassword(pws);
        createWalletBean.setSingleAddress(creatWalletRedChechbox.isChecked());
        Bundle bundle = new Bundle();
        bundle.putParcelable("CreateWalletBean", createWalletBean);
        start(BackupPurseFragment.class, bundle);
    }

}
