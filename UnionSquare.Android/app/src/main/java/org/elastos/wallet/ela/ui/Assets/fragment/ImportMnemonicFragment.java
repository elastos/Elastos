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


import android.support.v4.widget.NestedScrollView;
import android.support.v7.widget.AppCompatEditText;
import android.text.TextUtils;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;

import com.allen.library.SuperTextView;
import com.qmuiteam.qmui.layout.QMUIRelativeLayout;

import org.elastos.did.exception.DIDException;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.listener.RealmTransactionAbs;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonCreateSubWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.ImportMnemonicPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.mulwallet.CreatMulWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonCreateSubWalletViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.ImportMnemonicViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

/**
 * 助记词导入
 */
public class ImportMnemonicFragment extends BaseFragment implements ImportMnemonicViewData, CommonCreateSubWalletViewData, NewBaseViewData {


    @BindView(R.id.et_mnemonic)
    AppCompatEditText etMnemonic;
    @BindView(R.id.et_walletname)
    ClearEditText etWalletname;
    @BindView(R.id.et_walletpws)
    ClearEditText etWalletpws;
    @BindView(R.id.et_walletpws_agin)
    ClearEditText etWalletpwsAgin;
    @BindView(R.id.cb)
    CheckBox cb;
    @BindView(R.id.st_pws)
    SuperTextView stPws;
    @BindView(R.id.et_mnemonic_pwd)
    ClearEditText etMnemonicPwd;
    @BindView(R.id.qrl_pws)
    QMUIRelativeLayout qrlPws;
    ImportMnemonicPresenter importMnemonicPresenter;
    @BindView(R.id.nsv)
    NestedScrollView nsv;
    private String walletName;
    private String mnemonic;
    private String masterWalletID;
    private String payPassword;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_import_mnemonic;
    }


    @Override
    protected void initView(View view) {
        mRootView.setBackgroundResource(R.color.transparent);
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
        importMnemonicPresenter = new ImportMnemonicPresenter();
    }


    @OnClick({R.id.sb_sure})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.sb_sure:
                //确认导入钱包

                mnemonic = etMnemonic.getText().toString().trim();
                if (TextUtils.isEmpty(mnemonic)) {
                    showToastMessage(getString(R.string.mnemonicinputwrong));
                    return;
                }
                walletName = etWalletname.getText().toString().trim();
                if (TextUtils.isEmpty(walletName)) {
                    showToastMessage(getString(R.string.walletnamenotnull));
                    return;
                }
              /*  if (!MatcherUtil.isRightName(walletName)) {
                    showToastMessage(getString(R.string.walletnameruler));
                    return;
                }*/
                payPassword = etWalletpws.getText().toString().trim();
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
                String phrasePassword = etMnemonicPwd.getText().toString().trim();
                masterWalletID = AppUtlis.getStringRandom(8);
                importMnemonicPresenter.importWalletWithMnemonic(masterWalletID, mnemonic, phrasePassword,
                        payPassword, cb.isChecked(), this);
                break;

        }
    }

    @Override
    public void onImportMnemonic(String data) {
        new CommonCreateSubWalletPresenter().createSubWallet(masterWalletID, MyWallet.ELA, this, null);


    }

    String basecInfo;
    RealmUtil realmUtil = new RealmUtil();

    @Override
    public void onCreateSubWallet(String data, Object o) {

        if (data != null) {
            //创建Mainchain子钱包
            new CreatMulWalletPresenter().exportxPrivateKey(masterWalletID, payPassword, this);
            basecInfo = data;


        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "exportxPrivateKey":
                //保存did
                String privateKey = ((CommmonStringEntity) baseEntity).getData();
                try {
                    getMyDID().init(masterWalletID);//初始化mydid
                    getMyDID().getDidStore().initPrivateIdentity(privateKey, payPassword);
                    getMyDID().initDID(payPassword);
                } catch (DIDException e) {
                    e.printStackTrace();
                    showToast(getString(R.string.didinitfaile));
                }
                //通知首页
                Wallet masterWallet = realmUtil.updateWalletDetial(walletName, masterWalletID, basecInfo,getMyDID().getDidString());
                realmUtil.updateSubWalletDetial(masterWalletID, basecInfo, new RealmTransactionAbs() {
                    @Override
                    public void onSuccess() {
                        realmUtil.updateWalletDefault(masterWalletID, new RealmTransactionAbs() {
                            @Override
                            public void onSuccess() {
                                post(RxEnum.ONE.ordinal(), null, masterWallet);
                                toMainFragment();
                            }
                        });
                    }
                });
                break;
        }
    }
}
