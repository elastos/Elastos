package org.elastos.wallet.ela.ui.did.fragment;

import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.support.v7.widget.CardView;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.gson.JsonObject;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.presenter.ReceiptPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.ReceiptViewData;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.QRCodeUtils;
import org.elastos.wallet.ela.utils.ScreenUtil;
import org.elastos.wallet.ela.utils.ShareUtil;

import butterknife.BindView;
import butterknife.OnClick;

public class DIDCardFragment extends BaseFragment implements ReceiptViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_share)
    ImageView ivShare;
    @BindView(R.id.cardview)
    CardView cardview;
    @BindView(R.id.tv_didname)
    TextView tvDidname;
    @BindView(R.id.iv_qr)
    ImageView ivQr;
    @BindView(R.id.checkbox)
    CheckBox checkbox;
    private String didString;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_didcard;
    }

    @Override
    protected void setExtraData(Bundle data) {

        String didName = data.getString("didName");
        tvDidname.setText(didName);
        String walletId = data.getString("walletId");
        didString = data.getString("didString");
        new ReceiptPresenter().createAddress(walletId, MyWallet.ELA, this);

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.didcard);
    }

    @Override
    public void onCreateAddress(String data) {
        JsonObject object = new JsonObject();
        object.addProperty("address", data);
        object.addProperty("didString", didString);
        setQr(object.toString());

        checkbox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (!isChecked) {
                    object.remove("address");
                    setQr(object.toString());
                } else {
                    object.addProperty("address", data);
                    setQr(object.toString());
                }
            }
        });
    }

    private void setQr(String content) {
        QRCodeUtils.createQrCodeLogoBitmap(((BitmapDrawable) getResources().getDrawable(R.mipmap.did_qrcode_avatar)).getBitmap(), content, ScreenUtil.dp2px(getContext(), 240), ScreenUtil.dp2px(getContext(), 240), Constant.DIDCARD, MyWallet.ELA, -1, new QRCodeUtils.QRCodeCreateListener() {
            @Override
            public void onCreateSuccess(Bitmap bitmap) {
                ivQr.setImageBitmap(bitmap);
            }
        });
    }

    @OnClick({R.id.iv_share})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_share:
                ShareUtil.fxPic(getBaseActivity(), cardview);
                break;
        }
    }

}
