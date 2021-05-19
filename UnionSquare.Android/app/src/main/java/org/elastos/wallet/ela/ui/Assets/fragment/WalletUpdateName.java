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
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class WalletUpdateName extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    Unbinder unbinder;
    @BindView(R.id.et_name)
    EditText etName;
    Unbinder unbinder1;
    private String walletId;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_wallet_updatename;
    }

    @Override
    protected void setExtraData(Bundle data) {
        walletId = data.getString("walletId");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.modifywallletname));
    }

    @OnClick({R.id.tv_modify})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_modify:
                String walletName = etName.getText().toString().trim();
                if (TextUtils.isEmpty(walletName)) {
                    showToastMessage(getString(R.string.walletnamenotnull));
                    return;
                }

                new RealmUtil().upDataWalletName(walletId, walletName);
                post(RxEnum.UPDATA_WALLET_NAME.ordinal(), walletName, walletId);
                showToastMessage(getString(R.string.modifysucess));
                popBackFragment();
                break;

        }
    }


}
