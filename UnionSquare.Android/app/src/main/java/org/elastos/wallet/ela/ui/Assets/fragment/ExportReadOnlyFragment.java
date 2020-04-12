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
import org.elastos.wallet.ela.ui.Assets.presenter.WalletManagePresenter;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.QRCodeUtils;
import org.elastos.wallet.ela.utils.ScreenUtil;

import butterknife.BindView;
import butterknife.Unbinder;

public class ExportReadOnlyFragment extends BaseFragment implements CommmonStringWithMethNameViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_qr)
    ImageView ivQr;
    Unbinder unbinder;
    private Wallet wallet;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_exportreadomly;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        wallet = data.getParcelable("wallet");
    }

    @Override
    protected void initView(View view) {
        new WalletManagePresenter().exportReadonlyWallet(wallet.getWalletId(), this);
        tvTitle.setText(R.string.exportreadonly);

    }


    @Override
    public void onGetCommonData(String methodname, String data) {
        Bitmap mBitmap = QRCodeUtils.createQrCodeBitmap(data, ScreenUtil.dp2px(getContext(), 240), ScreenUtil.dp2px(getContext(), 240), Constant.CREATEREADONLY, null, -1);
        ivQr.setImageBitmap(mBitmap);
    }
}
