package org.elastos.wallet.ela.ui.Assets.fragment.transfer;

import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.support.v4.view.ViewPager;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.activity.PwdActivity;
import org.elastos.wallet.ela.ui.Assets.adapter.SignViewPagetAdapter;
import org.elastos.wallet.ela.ui.Assets.presenter.PwdPresenter;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.QRCodeUtils;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.ScreenUtil;
import org.elastos.wallet.ela.utils.ShareUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.elastos.wallet.ela.utils.widget.ScaleTransformer;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

public class SignFragment extends BaseFragment implements CommmonStringWithMethNameViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_publish)
    TextView tvPublish;
    @BindView(R.id.tv_vptitle)
    TextView tvVptitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.iv_qr)
    ImageView ivQr;
    @BindView(R.id.viewpage)
    ViewPager viewpage;
    @BindView(R.id.ll_vp)
    LinearLayout llVp;

    private Wallet wallet;
    private String signData;
    private String chainID;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_sign;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        wallet = data.getParcelable("wallet");
        String attribute = data.getString("attribute");
        int type = data.getInt("type");
        if (type == Constant.PUBLISH) {
            //签过名了
            showUI(attribute);
            return;
        }
        //未签名
        try {

            JsonObject JsonAttribute = new JsonParser().parse(attribute).getAsJsonObject();
            Intent intent = new Intent(getBaseActivity(), PwdActivity.class);
            intent.putExtra("wallet", wallet);
            intent.putExtra("chainId", JsonAttribute.get("ChainID").getAsString());
            intent.putExtra("attributes", attribute);
            intent.putExtra("type", 1);
            startActivity(intent);
        } catch (Exception e) {
            showToast(getString(R.string.error_30000));
        }
        registReceiver();
    }


    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.signedtreat));
        ivTitleRight.setImageResource(R.mipmap.top_share);
        ivTitleRight.setVisibility(View.VISIBLE);
    }

    @OnClick({R.id.iv_title_right, R.id.tv_publish})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_title_right:
                //截图分享
                ShareUtil.fxPic(getBaseActivity(), mRootView);
                break;
            case R.id.tv_publish:
                new PwdPresenter().publishTransaction(wallet.getWalletId(), chainID, signData, this);
                break;
        }
    }


    @Override
    public void onGetCommonData(String methodname, String data) {
        new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
            @Override
            public void affireBtnClick(View view) {

            }
        });
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.SIGNSUCCESS.ordinal()) {
            String signData = (String) result.getObj();
            showUI(signData);
        }
    }

    private void showUI(String signData) {
        this.signData = signData;
        JSONObject json = JSON.parseObject(signData);
        chainID = json.getString("ChainID");
        tvPublish.setVisibility(View.VISIBLE);
        setQr(signData);
    }

    public void setQr(String data) {
        //encodeTransaction  加密后的结果
        List<Bitmap> images = QRCodeUtils.createMulQrCodeBitmap(data, ScreenUtil.dp2px(getContext(), 170)
                , ScreenUtil.dp2px(getContext(), 170), Constant.PUBLISH);
        if (images.size() == 1) {
            ivQr.setVisibility(View.VISIBLE);
            llVp.setVisibility(View.GONE);
            tvVptitle.setVisibility(View.GONE);
            ivQr.setImageBitmap(images.get(0));
            return;
        }
        ivQr.setVisibility(View.GONE);
        llVp.setVisibility(View.VISIBLE);
        tvVptitle.setVisibility(View.VISIBLE);
        SignViewPagetAdapter signViewPagetAdapter = new SignViewPagetAdapter(images, getContext());
        viewpage.setAdapter(signViewPagetAdapter);
        viewpage.setPageTransformer(true, new ScaleTransformer());
        viewpage.setPageMargin(10);
        tvVptitle.setText(1 + "/" + images.size());
        viewpage.addOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
            @Override
            public void onPageSelected(int position) {
                super.onPageSelected(position);
                tvVptitle.setText((position + 1) + "/" + images.size());
            }
        });
    }
}
