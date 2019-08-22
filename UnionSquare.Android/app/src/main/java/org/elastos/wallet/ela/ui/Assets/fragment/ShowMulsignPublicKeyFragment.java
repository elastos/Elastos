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
        //{"CoinInfoList":[{"ChainID":"ELA","EarliestPeerTime":1561716528,"FeePerKB":10000,"VisibleAssets":["a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0"]}],"OwnerPubKey":"03d916c2072fd8fb57224e9747e0f1e36a2c117689cedf39e0132f3cb4f8ee673d","SingleAddress":false,"m":1,"mnemonicHasPassphrase":false,"n":1,"network":"","publicKeyRing":[{"requestPubKey":"0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280","xPubKey":"xpub6D5r16bFTY3FfNht7kobqQzkAHsUxzfKingYXXYUoTfNDSqCW2yjhHdt9yWRwtxx4zWoJ1m3pEo6hzQTswEA2UeEB16jEnYiHoDFwGH9c9z"}],"requestPubKey":"0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280","xPubKey":"xpub6D5r16bFTY3FfNht7kobqQzkAHsUxzfKingYXXYUoTfNDSqCW2yjhHdt9yWRwtxx4zWoJ1m3pEo6hzQTswEA2UeEB16jEnYiHoDFwGH9c9z"}
        JsonObject jsonData = new JsonParser().parse(data).getAsJsonObject();
        if (jsonData.has("requestPubKey")) {
            String requestPubKey = jsonData.get("requestPubKey").getAsString();
            Bitmap mBitmap = QRCodeUtils.createQrCodeBitmap(requestPubKey, ScreenUtil.dp2px(getContext(), 160), ScreenUtil.dp2px(getContext(), 160),Constant.CREATEMUL,null);
            ivQr.setImageBitmap(mBitmap);
            tvAddress.setText(requestPubKey);
        }
    }
}

