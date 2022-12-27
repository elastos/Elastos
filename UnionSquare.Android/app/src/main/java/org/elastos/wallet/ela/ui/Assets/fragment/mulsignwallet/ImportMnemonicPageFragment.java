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

package org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet;


import android.support.v7.widget.AppCompatEditText;
import android.text.TextUtils;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.TextView;

import com.allen.library.SuperTextView;
import com.qmuiteam.qmui.layout.QMUIRelativeLayout;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.CreateWalletBean;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 助记词导入
 */
public class ImportMnemonicPageFragment extends BaseFragment {


    @BindView(R.id.et_mnemonic)
    AppCompatEditText etMnemonic;
    @BindView(R.id.et_walletpws)
    ClearEditText etWalletpws;
    @BindView(R.id.et_walletpws_agin)
    ClearEditText etWalletpwsAgin;
    @BindView(R.id.st_pws)
    SuperTextView stPws;
    @BindView(R.id.et_mnemonic_pwd)
    ClearEditText etMnemonicPwd;
    @BindView(R.id.qrl_pws)
    QMUIRelativeLayout qrlPws;
    @BindView(R.id.tv_title)
    TextView tvTitle;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_import_mnemonicpage;
    }


    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.importmnemonic));
        stPws.setOnSuperTextViewClickListener(new SuperTextView.OnSuperTextViewClickListener() {
            @Override
            public void onClickListener(SuperTextView superTextView) {
                superTextView.setSwitchIsChecked(!superTextView.getSwitchIsChecked());
            }
        }).setSwitchCheckedChangeListener(new SuperTextView.OnSwitchCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    qrlPws.setVisibility(View.VISIBLE);
                } else {
                    qrlPws.setVisibility(View.GONE);
                }
            }
        });

    }


    @OnClick({R.id.sb_sure})
    public void onViewClicked(View view) {

        //确认导入钱包
        String mnemonic = etMnemonic.getText().toString().trim();
        if (TextUtils.isEmpty(mnemonic)) {
            showToastMessage(getString(R.string.mnemonicinputwrong));
            return;
        }

        String payPassword = etWalletpws.getText().toString().trim();
        if (TextUtils.isEmpty(payPassword)) {
            showToastMessage(getString(R.string.pwdnoempty));
            return;
        }
        if (!AppUtlis.chenckString(payPassword)) {
            showToast(getString(R.string.mmgsbd));
            return;
        }
        String walletPwdAgin = etWalletpwsAgin.getText().toString().trim();

        if (TextUtils.isEmpty(walletPwdAgin)) {
            showToastMessage(getString(R.string.aginpwdnotnull));
            return;
        }
        if (!payPassword.equals(walletPwdAgin)) {
            showToastMessage(getString(R.string.keynotthesame));
            return;
        }
        String phrasePassword = "";
        if (stPws.getSwitchIsChecked()) {
            phrasePassword = etMnemonicPwd.getText().toString().trim();
            if (TextUtils.isEmpty(phrasePassword)) {
                showToast(getString(R.string.please_enter_your_mnemonic_password_current_wallet));
                return;
            }

        }
        CreateWalletBean createWalletBean = new CreateWalletBean();
        createWalletBean.setPhrasePassword(phrasePassword);
        createWalletBean.setPayPassword(payPassword);
        createWalletBean.setMnemonic(mnemonic);
        post(RxEnum.IMPORTRIVATEKEY.ordinal(), null, createWalletBean);
        popTo(CreateMulWalletFragment.class, false);
    }


}
