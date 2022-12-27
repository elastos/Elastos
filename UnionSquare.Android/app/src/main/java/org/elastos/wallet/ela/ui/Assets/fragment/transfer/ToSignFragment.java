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

import android.graphics.Bitmap;
import android.os.Bundle;
import android.support.v4.view.ViewPager;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.adapter.SignViewPagetAdapter;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.QRCodeUtils;
import org.elastos.wallet.ela.utils.ScreenUtil;
import org.elastos.wallet.ela.utils.ShareUtil;
import org.elastos.wallet.ela.utils.widget.ScaleTransformer;

import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 备用
 */
public class ToSignFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_multip)
    TextView tvMultip;
    @BindView(R.id.tv_onlycode)
    TextView tvOnlycode;
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
    @BindView(R.id.tv_back)
    TextView tvBack;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_tosign;
    }

    @Override
    protected void setExtraData(Bundle data) {
        String attributes = data.getString("attributes");
        setData(attributes);
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.waitingsign));
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.top_share);


    }

    @OnClick({R.id.iv_title_right, R.id.tv_back})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_title_right:
                //截图分享
                ShareUtil.fxPic(getBaseActivity(), mRootView);
                break;
            case R.id.tv_back:
                toMainFragment();
                break;
        }
    }


    public void setData(String data) {
        //encodeTransaction  加密后的结果
        List<Bitmap> images = QRCodeUtils.createMulQrCodeBitmap(data, ScreenUtil.dp2px(getContext(), 250)
                , ScreenUtil.dp2px(getContext(), 250), Constant.SIGN, -1);
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
            JSONObject JsonAttribute = JSON.parseObject(data);
            String msg = String.format(getString(R.string.onlyid), JsonAttribute.getString("ID"));
            tvOnlycode.setText(msg);
            tvOnlycode.setVisibility(View.VISIBLE);
            tvMultip.setVisibility(View.VISIBLE);
        } catch (Exception e) {
        }
        ivQr.setVisibility(View.GONE);
        llVp.setVisibility(View.VISIBLE);
        tvVptitle.setVisibility(View.VISIBLE);
        tvMultip.setVisibility(View.VISIBLE);
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
