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

import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.CreateWalletBean;
import org.elastos.wallet.ela.ui.Assets.fragment.BackupPurseFragment;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.widget.keyboard.SecurityEditText;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class CreatePrivateKey extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.et_walletpws)
    SecurityEditText etWalletpws;
    @BindView(R.id.et_walletpws_next)
    SecurityEditText etWalletpwsNext;

    @Override
    protected int getLayoutId() {
        return R.layout.createprivatekey;
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.createprivatekey));
    }

    @OnClick({R.id.tv_create_wallet})
    public void onViewClick(View view) {
        switch (view.getId()) {
            case R.id.tv_create_wallet:
                createwallet();
                break;


        }
    }

    private void createwallet() {

        String pws = etWalletpws.getText().toString().trim();
        String pws_next = etWalletpwsNext.getText().toString().trim();


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
        createWalletBean.setMasterWalletName("");
        createWalletBean.setPayPassword(pws);
        createWalletBean.setSingleAddress(false);
        Bundle bundle = new Bundle();
        bundle.putParcelable("CreateWalletBean", createWalletBean);
        bundle.putInt("openType", RxEnum.PRIVATEKEY.ordinal());
        start(BackupPurseFragment.class, bundle);
    }


}
