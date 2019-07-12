package org.elastos.wallet.ela.ui.Assets.fragment.transfer;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.gson.JsonObject;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.QRCodeUtils;
import org.elastos.wallet.ela.utils.ScreenUtil;
import org.elastos.wallet.ela.utils.ShareUtil;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class SignFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.iv_qr)
    ImageView ivQr;
    Unbinder unbinder;
    private String attributes;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_sign;
    }

    @Override
    protected void setExtraData(Bundle data) {
        attributes = data.getString("attributes");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.waitingsign));
        ivTitleRight.setImageResource(R.mipmap.top_share);
        JsonObject jsonObject = new JsonObject();
        jsonObject.addProperty("type", Constant.SIGN);
        jsonObject.addProperty("data", attributes);
        Bitmap mBitmap = QRCodeUtils.createQrCodeBitmap(jsonObject.toString(), ScreenUtil.dp2px(getContext(), 160), ScreenUtil.dp2px(getContext(), 160));
        ivQr.setImageBitmap(mBitmap);
    }

    @OnClick({R.id.iv_title_right})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_title_right:
                //截图分享
                ShareUtil.fxPic(getBaseActivity(), mRootView);
                break;
        }
    }

}
