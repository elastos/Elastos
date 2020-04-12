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

package org.elastos.wallet.ela.ui.Assets.fragment.transfer;

import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.support.v4.view.ViewPager;
import android.text.TextUtils;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
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
import org.json.JSONException;
import org.json.JSONObject;

import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

public class SignFragment extends BaseFragment implements CommmonStringWithMethNameViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_signstatus)
    TextView tvSignstatus;
    @BindView(R.id.tv_multip)
    TextView tvMultip;
    @BindView(R.id.tv_onlycode)
    TextView tvOnlycode;
    @BindView(R.id.tv_publish)
    TextView tvPublish;
    @BindView(R.id.tv_back)
    TextView tvBack;
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
    private PwdPresenter presenter;
    int transType;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_sign;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        wallet = data.getParcelable("wallet");
        signData = data.getString("attributes");
        transType = data.getInt("transType");
        setQr(signData);
        boolean signStatus = data.getBoolean("signStatus");
        if (signStatus) {
            showUI = true;
        }

        try {
            com.alibaba.fastjson.JSONObject JsonAttribute = JSON.parseObject(signData);
            chainID = JsonAttribute.getString("ChainID");
            //判断签名状态
            presenter = new PwdPresenter();
            presenter.getTransactionSignedInfo(wallet.getWalletId(), chainID, signData, this);
        } catch (Exception e) {

            e.printStackTrace();
        }

    }


    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.waitingsign));
        ivTitleRight.setImageResource(R.mipmap.top_share);
        ivTitleRight.setVisibility(View.VISIBLE);
    }

    @OnClick({R.id.iv_title_right, R.id.tv_publish, R.id.tv_back})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_title_right:
                //截图分享
                ShareUtil.fxPic(getBaseActivity(), mRootView);
                break;
            case R.id.tv_back:
                toMainFragment();
                break;
            case R.id.tv_publish:
                new PwdPresenter().publishTransaction(wallet.getWalletId(), chainID, signData, this);
                break;
        }
    }


    @Override
    public void onGetCommonData(String methodname, String data) {
        switch (methodname) {
            case "publishTransaction":
                new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        popBackFragment();
                    }
                });
                String hash = "";
                try {
                    JSONObject pulishdata = new JSONObject(data);
                    hash = pulishdata.getString("TxHash");
                } catch (JSONException e) {
                    e.printStackTrace();
                }
                post(RxEnum.TRANSFERSUCESS.ordinal(), transType + "", hash);
                break;
            case "getTransactionSignedInfo":
                //判断签名状态
                if (TextUtils.isEmpty(data)) {
                    return;
                }
                JsonObject signJson = new JsonParser().parse(data).getAsJsonArray().get(0).getAsJsonObject();
                int N = 1;
                int M = 1;
                if (signJson.has("N")) {
                    N = signJson.get("N").getAsInt();
                }
                if (signJson.has("M")) {
                    M = signJson.get("M").getAsInt();
                }

                int signedNum;
                try {
                    signedNum = signJson.getAsJsonArray("Signers").size();
                } catch (Exception e) {
                    signedNum = 0;
                }

                if (N > 1) {
                    //多签签名
                    tvSignstatus.setVisibility(View.VISIBLE);
                    String msg = String.format(getString(R.string.signstatus), signedNum + "", M - signedNum + "");
                    tvSignstatus.setText(msg);
                }

                if (signedNum >= M) {
                    //签名完成  直接展示状态 所有钱包都可以publish  所有先判断它
                    tvTitle.setText(getString(R.string.signedtreat));
                    setQr(signData);
                    tvBack.setVisibility(View.GONE);
                    tvPublish.setVisibility(View.VISIBLE);
                    return;
                } else if (showUI) {
                    //签名未完成 且本页面已经签了名 直接展示状态
                    setQr(signData);
                    return;
                } else if (wallet.getType() == 1 || wallet.getType() == 3) {
                    //0 普通单签 1单签只读 2普通多签 3多签只读
                    setQr(signData);
                    //showToast(getString(R.string.nopermiss));
                    return;
                }
                //去签名
                Intent intent = new Intent(getBaseActivity(), PwdActivity.class);
                intent.putExtra("wallet", wallet);
                intent.putExtra("chainId", chainID);
                intent.putExtra("attributes", signData);
                intent.putExtra("onlySign", true);
                intent.putExtra("transType", transType);
                startActivity(intent);
                registReceiver();
                break;

        }

    }

    private boolean showUI = false;

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.SIGNSUCCESS.ordinal()) {
            String signData = (String) result.getObj();
            //再次判断签名状态 用于直接绘制ui
            showUI = true;
            presenter.getTransactionSignedInfo(wallet.getWalletId(), chainID, signData, this);
            this.signData = signData;

        }
    }


    public void setQr(String data) {
        //encodeTransaction  加密后的结果
        List<Bitmap> images = QRCodeUtils.createMulQrCodeBitmap(data, ScreenUtil.dp2px(getContext(), 240)
                , ScreenUtil.dp2px(getContext(), 240), Constant.SIGN, transType);
        if (images.size() == 1) {
            ivQr.setVisibility(View.VISIBLE);
            llVp.setVisibility(View.GONE);
            tvVptitle.setVisibility(View.GONE);
            ivQr.setImageBitmap(images.get(0));
            tvOnlycode.setVisibility(View.GONE);
            tvMultip.setVisibility(View.GONE);
            return;
        }
        try {
            com.alibaba.fastjson.JSONObject JsonAttribute = JSON.parseObject(data);
            String msg = String.format(getString(R.string.onlyid), JsonAttribute.getString("ID"));
            tvOnlycode.setText(msg);
            tvOnlycode.setVisibility(View.VISIBLE);
            tvMultip.setVisibility(View.VISIBLE);
        } catch (Exception e) {
        }
        ivQr.setVisibility(View.GONE);
        llVp.setVisibility(View.VISIBLE);
        tvVptitle.setVisibility(View.VISIBLE);
        SignViewPagetAdapter signViewPagetAdapter = new SignViewPagetAdapter(images, getContext());
        viewpage.setAdapter(signViewPagetAdapter);
        viewpage.setPageTransformer(true, new ScaleTransformer());
        viewpage.setPageMargin(30);
        String msg = String.format(getString(R.string.currentpage), 1 + "/" + images.size());
        tvVptitle.setText(msg);
        viewpage.addOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
            @Override
            public void onPageSelected(int position) {
                super.onPageSelected(position);
                String msg = String.format(getString(R.string.currentpage), (position + 1) + "/" + images.size());
                tvVptitle.setText(msg);
            }
        });
    }
}
