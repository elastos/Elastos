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

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet.CreateMulWalletFragment;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.QrBean;
import org.elastos.wallet.ela.utils.ScanQRcodeUtil;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class ChoseCreateWalletFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    Unbinder unbinder;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_chosecreate;
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.create_a_wallet));
    }

    @OnClick({R.id.rl_single, R.id.rl_single_readomly, R.id.rl_mul})
    public void onClicked(View view) {
        switch (view.getId()) {
            case R.id.rl_single:
                start(CreateWalletFragment.class);
                break;
            case R.id.rl_single_readomly:
                requstPermissionOk();
                break;
            case R.id.rl_mul:
                //多签钱包
                start(CreateMulWalletFragment.class);
                break;
        }
    }

    @Override
    protected void requstPermissionOk() {
        new ScanQRcodeUtil().scanQRcode(this);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        //处理扫描结果（在界面上显示）
        if (resultCode == RESULT_OK && requestCode == ScanQRcodeUtil.SCAN_QR_REQUEST_CODE && data != null) {
            String result = data.getStringExtra("result");//&& matcherUtil.isMatcherAddr(result)
            Bundle bundle = new Bundle();
            bundle.putString("result", result);
            try {
                QrBean qrBean = JSON.parseObject(result, QrBean.class);
                int type = qrBean.getExtra().getType();
                if (type != Constant.CREATEREADONLY) {
                    showToast(getString(R.string.infoformatwrong));
                } else {
                    start(CreateSignReadOnlyWalletFragment.class, bundle);
                }
            } catch (Exception e) {
                showToast(getString(R.string.infoformatwrong));
            }

        }

    }
}
