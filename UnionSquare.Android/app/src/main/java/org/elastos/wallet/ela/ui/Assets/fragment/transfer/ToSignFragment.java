package org.elastos.wallet.ela.ui.Assets.fragment.transfer;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.support.v4.view.ViewPager;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.gson.JsonObject;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.adapter.SignViewPagetAdapter;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.QRCodeUtils;
import org.elastos.wallet.ela.utils.ScreenUtil;
import org.elastos.wallet.ela.utils.ShareUtil;
import org.elastos.wallet.ela.utils.widget.ScaleTransformer;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class ToSignFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_vptitle)
    TextView tvVptitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.iv_qr)
    ImageView ivQr;
    @BindView(R.id.viewpage)
    ViewPager viewpage;
    private String attributes;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_tosign;
    }

    @Override
    protected void setExtraData(Bundle data) {
        attributes = data.getString("attributes");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.waitingsign));
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.top_share);
        JsonObject jsonObject = new JsonObject();
        jsonObject.addProperty("type", Constant.SIGN);
        jsonObject.addProperty("data", attributes);
        Bitmap mBitmap = QRCodeUtils.createQrCodeBitmap(jsonObject.toString(), ScreenUtil.dp2px(getContext(), 170), ScreenUtil.dp2px(getContext(), 170));
        ivQr.setImageBitmap(mBitmap);
        List<Bitmap> images = new ArrayList<>();
        images.add(mBitmap);
        images.add(mBitmap);
        images.add(mBitmap);
        SignViewPagetAdapter signViewPagetAdapter = new SignViewPagetAdapter(images, getContext());
        viewpage.setAdapter(signViewPagetAdapter);
        viewpage.setPageTransformer(true, new ScaleTransformer());
        viewpage.setPageMargin(10);
        tvVptitle.setText(1 +"/"+ images.size());
        viewpage.addOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
            @Override
            public void onPageSelected(int position) {
                super.onPageSelected(position);
                tvVptitle.setText((position + 1) +"/"+ images.size());
            }
        });
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
