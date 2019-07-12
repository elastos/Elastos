package org.elastos.wallet.ela.ui.Assets.fragment;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.presenter.WallletManagePresenter;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.QRCodeUtils;
import org.elastos.wallet.ela.utils.ScreenUtil;

import butterknife.BindView;
import butterknife.OnClick;

public class ShowMulsignPublicKeyFragment extends BaseFragment implements CommmonStringWithMethNameViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_qr)
    ImageView ivQr;
    @BindView(R.id.tv_address)
    TextView tvAddress;
    private Wallet wallet;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_showmulsignpublickey;
    }

    @Override
    protected void setExtraData(Bundle data) {
        wallet = data.getParcelable("wallet");

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.mulsignpiblickey));
        new WallletManagePresenter().exportReadonlyWallet(wallet.getWalletId(), this);
    }

    @OnClick({R.id.tv_copy})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_copy:
                ClipboardUtil.copyClipboar(getBaseActivity(), tvAddress.getText().toString().trim());
                break;
        }
    }

    @Override
    public void onGetCommonData(String methodname, String data) {
        JsonObject jsonData = new JsonParser().parse(data).getAsJsonObject();


        if (jsonData.has("requestPubKey")) {
            String requestPubKey = jsonData.get("requestPubKey").getAsString();
            JsonObject jsonObject = new JsonObject();
            jsonObject.addProperty("type", Constant.CREATEMUL);
            jsonObject.addProperty("data", requestPubKey);
            Bitmap mBitmap = QRCodeUtils.createQrCodeBitmap(jsonObject.toString(), ScreenUtil.dp2px(getContext(), 160), ScreenUtil.dp2px(getContext(), 160));
            ivQr.setImageBitmap(mBitmap);
            tvAddress.setText(requestPubKey);
        }
    }
}

