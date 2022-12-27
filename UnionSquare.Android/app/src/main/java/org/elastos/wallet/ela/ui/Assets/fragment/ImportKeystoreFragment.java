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


import android.app.Fragment;
import android.support.v7.widget.AppCompatEditText;
import android.text.TextUtils;
import android.view.View;

import com.allen.library.SuperButton;

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
import org.elastos.wallet.ela.ui.Assets.presenter.ImportKeystorePresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.mulwallet.CreatMulWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonCreateSubWalletViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.ImportKeystoreViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

/**
 * A simple {@link Fragment} subclass.
 */
public class ImportKeystoreFragment extends BaseFragment implements ImportKeystoreViewData, CommonCreateSubWalletViewData, NewBaseViewData {


    @BindView(R.id.et_keystore)
    AppCompatEditText etKeystore;
    @BindView(R.id.et_walletname)
    ClearEditText etWalletname;
    @BindView(R.id.et_walletpws)
    ClearEditText etWalletpws;
    @BindView(R.id.et_walletpws_agin)
    ClearEditText etWalletpwsAgin;
    @BindView(R.id.sb_sure)
    SuperButton sbSure;
    @BindView(R.id.et_keystore_pwd)
    ClearEditText etKeystorePwd;

    private ImportKeystorePresenter presenter;
    private String keystore;
    private String walletName;
    private String masterWalletID;
    private RealmUtil realmUtil;
    private String payPassword;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_keystore;
    }


    @Override
    protected void initView(View view) {
        mRootView.setBackgroundResource(R.color.transparent);
        realmUtil = new RealmUtil();
        presenter = new ImportKeystorePresenter();
    }

    @OnClick({R.id.sb_sure})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.sb_sure:
                //确认导入钱包

                keystore = etKeystore.getText().toString().trim();
                if (TextUtils.isEmpty(keystore)) {
                    showToastMessage(getString(R.string.keystoreinputwrong));
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
                String keystorePwd = etKeystorePwd.getText().toString().trim();

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

                masterWalletID = AppUtlis.getStringRandom(8);
                presenter.importWalletWithKeystore(masterWalletID, keystore, keystorePwd,
                        payPassword, this);
                break;

        }
    }

    @Override
    public void onImportKeystore(String data) {
        new CommonCreateSubWalletPresenter().createSubWallet(masterWalletID, MyWallet.ELA, this,null);
    }

    String basecInfo;

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


