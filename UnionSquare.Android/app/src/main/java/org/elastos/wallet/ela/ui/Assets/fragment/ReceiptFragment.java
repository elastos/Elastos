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
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.presenter.ReceiptPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.ReceiptViewData;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.QRCodeUtils;
import org.elastos.wallet.ela.utils.ScreenUtil;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class ReceiptFragment extends BaseFragment implements ReceiptViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_qr)
    ImageView ivQr;
    @BindView(R.id.tv_address)
    TextView tvAddress;
    Unbinder unbinder;
    @BindView(R.id.tv_wallet_list)
    TextView tvWalletList;
    private Wallet wallet;
    private String chainId;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_receip;
    }

    @Override
    protected void setExtraData(Bundle data) {
        wallet = data.getParcelable("wallet");
        chainId = data.getString("ChainId", "ELA");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.collection));
        if (wallet.isSingleAddress()) {
            //单地址
            tvWalletList.setVisibility(View.INVISIBLE);
        }


        new ReceiptPresenter().createAddress(wallet.getWalletId(), chainId, this);
    }

    @OnClick({R.id.tv_copy, R.id.tv_wallet_list})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_copy:
                ClipboardUtil.copyClipboar(getBaseActivity(), tvAddress.getText().toString().trim());
                break;
            case R.id.tv_wallet_list:
                Bundle bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                bundle.putString("ChainId", chainId);
                start(AddressListFragment.class, bundle);
                break;

        }
    }

    @Override
    public void onCreateAddress(String data) {
        tvAddress.setText(data);
        Bitmap mBitmap = QRCodeUtils.createQrCodeBitmap(data, ScreenUtil.dp2px(getContext(), 240), ScreenUtil.dp2px(getContext(), 240));
        ivQr.setImageBitmap(mBitmap);
    }
}
