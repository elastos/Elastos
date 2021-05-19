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

package org.elastos.wallet.ela.ui.Assets.activity;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.presenter.PwdPresenter;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.AndroidWorkaround;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.RxEnum;
import org.json.JSONException;
import org.json.JSONObject;

import butterknife.BindView;
import butterknife.OnClick;

public class PwdActivity extends BaseActivity implements CommmonStringWithMethNameViewData {
    @BindView(R.id.et_pwd)
    ClearEditText etPwd;
    private Wallet wallet;
    private String chainId;
    private PwdPresenter presenter;
    private String attributes;
    private String pwd, openType;
    private boolean onlySign;//为1时候只需要签名然后post出去
    private int transType;

    @Override
    protected int getLayoutId() {
        if (AndroidWorkaround.checkDeviceHasNavigationBar(this)) {
            AndroidWorkaround.assistActivity(findViewById(android.R.id.content));
        }
        return R.layout.activity_pwd;
    }

    @Override
    protected void initView() {

        getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        //一定要在setContentView之后调用，否则无效
        getWindow().setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        presenter = new PwdPresenter();
    }

    @Override
    protected void setExtraData(Intent data) {
        openType = data.getStringExtra("openType");
        chainId = data.getStringExtra("chainId");
        attributes = data.getStringExtra("attributes");
        wallet = data.getParcelableExtra("wallet");
        onlySign = data.getBooleanExtra("onlySign", false);//onlySign签名成功不publish
        transType = data.getIntExtra("transType", 13);


      /*  tvAddress.setText(toAddress);
        tvAmount.setText(amount + " " + chainId);
        tvCharge.setText(NumberiUtil.maxNumberFormat(new BigDecimal(((double) fee) / MyWallet.RATE + "").toPlainString(), 12) + " " + chainId);//0.0001
        llRate.setVisibility(View.GONE);*/
    }

    @OnClick({R.id.tv_sure})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sure:
                //确定
                pwd = etPwd.getText().toString().trim();
                if (TextUtils.isEmpty(pwd)) {
                    showToastMessage(getString(R.string.pwdnoempty));
                    return;
                }
                presenter.signTransaction(wallet.getWalletId(), chainId, attributes, pwd, this);
                break;

        }
    }


    @Override
    public void onGetCommonData(String methodname, String data) {
        switch (methodname) {
            case "signTransaction":
                if (onlySign) {
                    post(RxEnum.SIGNSUCCESS.ordinal(), "签名成功", data);
                    finish();
                    return;
                }
                switch (wallet.getType()) {
                    //0 普通单签 1单签只读 2普通多签 3多签只读
                    case 0:
                        presenter.publishTransaction(wallet.getWalletId(), chainId, data, this);
                        break;
                    case 2:
                        post(RxEnum.SIGNSUCCESS.ordinal(), "", data);
                        finish();
                        break;

                }

                break;
            case "publishTransaction":
                String hash = "";
                try {
                    JSONObject pulishdata = new JSONObject(data);
                    hash = pulishdata.getString("TxHash");
                } catch (JSONException e) {
                    e.printStackTrace();
                }
                post(RxEnum.TRANSFERSUCESS.ordinal(), transType + "", hash);
                post(RxEnum.TRANSACTIONSUCCESSMESSAGE.ordinal(), openType, null);//只为通知特定页面加的补丁
                finish();
                break;
        }
    }
}
