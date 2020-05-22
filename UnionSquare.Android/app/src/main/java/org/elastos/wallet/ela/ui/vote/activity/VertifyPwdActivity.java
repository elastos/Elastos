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

package org.elastos.wallet.ela.ui.vote.activity;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.presenter.VertifyPwdPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.utils.AndroidWorkaround;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 为了验证密码单独存在
 */
public class VertifyPwdActivity extends BaseActivity implements NewBaseViewData {
    @BindView(R.id.et_pwd)
    ClearEditText etPwd;

    private String walletId, openType;


    @Override
    protected int getLayoutId() {
        if (AndroidWorkaround.checkDeviceHasNavigationBar(this)) {
            AndroidWorkaround.assistActivity(findViewById(android.R.id.content));
        }
        return R.layout.activity_other_pwd;
    }

    @Override
    protected void initView() {

        getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        //一定要在setContentView之后调用，否则无效
        getWindow().setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);

    }

    @Override
    protected void setExtraData(Intent data) {
        walletId = data.getStringExtra("walletId");
        openType = data.getStringExtra("openType");//区分谁打开的classsimplename
    }


    @OnClick({R.id.tv_sure})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sure:
                //确定
                String pwd = etPwd.getText().toString().trim();
                if (TextUtils.isEmpty(pwd)) {
                    showToastMessage(getString(R.string.pwdnoempty));
                    return;
                }
                new VertifyPwdPresenter().verifyPayPassword(walletId, pwd, this);

                break;

        }
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

        switch (methodName) {
            case "verifyPayPassword":

                boolean result = ((CommmonBooleanEntity) baseEntity).getData();
                String pwd= (String) o;
                if (result) {
                    post(RxEnum.VERTIFYPAYPASS.ordinal(), openType, pwd);
                    finish();
                } else {
                    showToastMessage(getString(R.string.error_20003));
                }

                break;

        }
    }

}


