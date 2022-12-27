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
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.listener.RealmTransactionAbs;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonCreateSubWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.CreateMasterWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonCreateSubWalletViewData;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.QrBean;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;


public class CreateSignReadOnlyWalletFragment extends BaseFragment implements CommmonStringWithMethNameViewData, CommonCreateSubWalletViewData {
    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.et_walletname)
    ClearEditText etWalletname;
    private String result;
    private String masterWalletID;
    private String name;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_create_singlereadonly;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        result = data.getString("result");
        try {
            QrBean qrBean = JSON.parseObject(result, QrBean.class);
            result = qrBean.getData();
        } catch (Exception e) {
            //showToast(getString(R.string.error_20007));
        }

    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.createsinglereadonlywallet));
    }


    @OnClick({R.id.sb_create_wallet})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.sb_create_wallet:
                createwallet();
                break;

        }
    }


    private void createwallet() {
        name = etWalletname.getText().toString().trim();

        if (TextUtils.isEmpty(name)) {
            showToast(getString(R.string.inputWalletName));
            return;
        }
        if (TextUtils.isEmpty(result)) {
            showToast(getString(R.string.error_20001));
            return;
        }
        masterWalletID = AppUtlis.getStringRandom(8);
        new CreateMasterWalletPresenter().importReadonlyWallet(masterWalletID, result, this);
    }

    @Override
    public void onGetCommonData(String methodname, String data) {
        if (data != null) {
            new CommonCreateSubWalletPresenter().createSubWallet(masterWalletID, MyWallet.ELA, this, null);
        }

    }

    @Override
    public void onCreateSubWallet(String data, Object view) {
        if (data != null) {
            //创建Mainchain子钱包
            RealmUtil realmUtil = new RealmUtil();
            Wallet masterWallet = realmUtil.updateWalletDetial(name, masterWalletID, data);
            realmUtil.updateSubWalletDetial(masterWalletID, data, new RealmTransactionAbs() {
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


        }
    }
}
