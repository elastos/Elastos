package org.elastos.wallet.ela.ui.Assets.fragment;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.gson.JsonObject;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.presenter.WallletManagePresenter;
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
        new WallletManagePresenter().exportReadonlyWallet(wallet.getWalletId(), this);
        tvTitle.setText(R.string.exportreadonly);

    }


    @Override
    public void onGetCommonData(String methodname, String data) {
        Bitmap mBitmap = QRCodeUtils.createQrCodeBitmap(data, ScreenUtil.dp2px(getContext(), 180), ScreenUtil.dp2px(getContext(), 180), Constant.CREATEREADONLY,null);
        ivQr.setImageBitmap(mBitmap);
    }
}
