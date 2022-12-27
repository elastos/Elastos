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
import android.support.v7.widget.AppCompatTextView;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.View;
import android.view.WindowManager;
import android.widget.CompoundButton;
import android.widget.TextView;

import com.allen.library.SuperTextView;
import com.qmuiteam.qmui.layout.QMUILinearLayout;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.CreateWalletBean;
import org.elastos.wallet.ela.ui.Assets.presenter.GenerateMnemonicPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.GenerateMnemonicData;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.widget.keyboard.SecurityEditText;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 助记词
 */
public class MnemonicWordFragment extends BaseFragment implements GenerateMnemonicData {


    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.st_pws)
    SuperTextView stPws;
    @BindView(R.id.et_walletpws)
    SecurityEditText etWalletpws;
    @BindView(R.id.et_walletpws_next)
    SecurityEditText etWalletpwsNext;
    @BindView(R.id.ll_mnemonic_pws)
    QMUILinearLayout llMnemonicPws;
    @BindView(R.id.ll_mnemonic_word)
    QMUILinearLayout ll_mnemonic_word;
    public boolean Checked = false;
    @BindView(R.id.tv_mnemonic)
    AppCompatTextView tv_mnemonic;
    private CreateWalletBean createWalletBean;
    private Bundle bundle;


    @Override
    protected int getLayoutId() {
        getBaseActivity().getWindow().setFlags(WindowManager.LayoutParams.FLAG_SECURE, WindowManager.LayoutParams.FLAG_SECURE);
        return R.layout.fragment_mnemonic_word;
    }

    @Override
    protected void setExtraData(Bundle data) {
        bundle = data;
        createWalletBean = data.getParcelable("CreateWalletBean");

    }

    @Override
    protected void initInjector() {
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.mnemonic));
        //开启自定义键盘
        // AppUtlis.securityKeyboard(ll_mnemonic_word);
        //开关
        stPws.setOnSuperTextViewClickListener(new SuperTextView.OnSuperTextViewClickListener() {
            @Override
            public void onClickListener(SuperTextView superTextView) {
                superTextView.setSwitchIsChecked(!superTextView.getSwitchIsChecked());
            }
        }).setSwitchCheckedChangeListener(new SuperTextView.OnSwitchCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                Checked = isChecked;
                if (isChecked) {
                    llMnemonicPws.setVisibility(View.VISIBLE);
                } else {
                    llMnemonicPws.setVisibility(View.GONE);
                }
            }
        });

        String type = (1 == new SPUtil(getContext()).getLanguage()) ? "english" : "chinese";
        //创建助记词
        new GenerateMnemonicPresenter().generateMnemonic(type, this);


    }


    @OnClick({R.id.sb_create_wallet})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.sb_create_wallet:
                //选了助记词密码后的操作
                String pws = "";
                if (Checked) {
                    pws = etWalletpws.getText().toString().trim();
                    String pws_next = etWalletpwsNext.getText().toString().trim();
                    if (TextUtils.isEmpty(tv_mnemonic.getText().toString().trim())) {

                        return;
                    }
                    if (TextUtils.isEmpty(pws)) {
                        showToast(getString(R.string.please_enter_your_mnemonic_password));
                        return;
                    }

                    if (TextUtils.isEmpty(pws_next)) {
                        showToast(getString(R.string.inputWalltPwdAgin));
                        return;
                    }

                    if (!AppUtlis.chenckString(pws_next)) {
                        showToast(getString(R.string.mmgsbd));
                        return;
                    }
                    if (!pws.equals(pws_next)) {
                        showToast(getString(R.string.lcmmsrbyz));
                        return;
                    }
                }
                createWalletBean.setPhrasePassword(pws);
                toNextPager();
                break;
        }
    }

    private void toNextPager() {
        bundle.putString("mnemonic", createWalletBean.getMnemonic());
        bundle.putParcelable("createWalletBean", createWalletBean);
        start(VerifyMnemonicWordsFragment.class, bundle);
    }


    //正则去掉逗号 括号
    private String regex(String at) {
        return at.replaceAll("\\[", "").
                replaceAll("\\]", "").replaceAll(",", "");
    }


    @Override
    public void onGetMneonic(String mnemonic) {
        if (mnemonic == null) {
            showToastMessage("助记词生成失败");
            return;
        }
        createWalletBean.setMnemonic(mnemonic);
        String masterWalletID = AppUtlis.getStringRandom(8);
        createWalletBean.setMasterWalletID(masterWalletID);
        //  String[] re = mnemonic.split(" ");//用split()函数直接分割
        tv_mnemonic.setText(mnemonic);
        /*tv_mnemonic.setText(regex(Arrays.toString(Arrays.copyOfRange(re, 0, 8))) + "\n\n" +
                regex(Arrays.toString(Arrays.copyOfRange(re, 8, re.length))));*/

    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        getBaseActivity().getWindow().clearFlags(WindowManager.LayoutParams.FLAG_SECURE);
    }

}
