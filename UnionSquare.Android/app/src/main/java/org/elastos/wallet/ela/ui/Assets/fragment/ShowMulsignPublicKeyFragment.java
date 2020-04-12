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

import android.graphics.Bitmap;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.QRCodeUtils;
import org.elastos.wallet.ela.utils.ScreenUtil;

import butterknife.BindView;
import butterknife.OnClick;

public class ShowMulsignPublicKeyFragment extends BaseFragment/* implements NewBaseViewData */{
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_qr)
    ImageView ivQr;
    @BindView(R.id.tv_address)
    TextView tvAddress;
    private Wallet wallet;
    private String requestPubKey;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_showmulsignpublickey;
    }

    @Override
    protected void setExtraData(Bundle data) {
        wallet = data.getParcelable("wallet");
        requestPubKey = data.getString("requestPubKey");
        Bitmap mBitmap = QRCodeUtils.createQrCodeBitmap(requestPubKey, ScreenUtil.dp2px(getContext(), 240), ScreenUtil.dp2px(getContext(), 240), Constant.CREATEMUL, null,-1);
        ivQr.setImageBitmap(mBitmap);
        tvAddress.setText(requestPubKey);

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.mulsignpiblickey));
        // new WalletManagePresenter().getPubKeyInfo(wallet.getWalletId(), this);
    }

    @OnClick({R.id.tv_copy})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_copy:
                ClipboardUtil.copyClipboar(getBaseActivity(), tvAddress.getText().toString().trim());
                break;
        }
    }


   /* @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        String data = ((CommmonStringEntity) baseEntity).getData();
        JsonObject jsonData = new JsonParser().parse(data).getAsJsonObject();
        String derivationStrategy = jsonData.get("derivationStrategy").getAsString();
        int n = jsonData.get("n").getAsInt();
        String requestPubKey;
        if ("BIP44".equals(derivationStrategy) && n > 1) {
            requestPubKey = jsonData.get("xPubKey").getAsString();

        } else {
            requestPubKey = jsonData.get("xPubKeyHDPM").getAsString();
        }
        if (!TextUtils.isEmpty(requestPubKey)) {
            Bitmap mBitmap = QRCodeUtils.createQrCodeBitmap(requestPubKey, ScreenUtil.dp2px(getContext(), 160), ScreenUtil.dp2px(getContext(), 160), Constant.CREATEMUL, null);
            ivQr.setImageBitmap(mBitmap);
            tvAddress.setText(requestPubKey);
        }
    }*/
}

