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
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.utils.AndroidWorkaround;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 输入投票数量
 */
public class VoteActivity extends BaseActivity {
    @BindView(R.id.et_pwd)
    ClearEditText etPwd;
    String maxBalance;
    @BindView(R.id.tv_max)
    TextView tv_max;
    @BindView(R.id.tv_maxvote)
    TextView tv_maxvote;

    @Override
    protected int getLayoutId() {
        if (AndroidWorkaround.checkDeviceHasNavigationBar(this)) {
            AndroidWorkaround.assistActivity(findViewById(android.R.id.content));
        }
        return R.layout.activity_vote;
    }

    @Override
    protected void initView() {
        getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        //一定要在setContentView之后调用，否则无效
        getWindow().setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);

    }

    @Override
    protected void setExtraData(Intent data) {
        maxBalance = data.getStringExtra("maxBalance");
        //Double num = Double.parseDouble(fee) / MyWallet.RATE;
        tv_max.setOnClickListener(v -> etPwd.setText("MAX"));
        tv_maxvote.setText(getString(R.string.maximum_voting_rights) + "  " + maxBalance);
        etPwd.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (!TextUtils.isEmpty(etPwd.getText()) && etPwd.getText().toString().equals("MAX")) {
                    etPwd.setText("");
                }
                return false;
            }
        });
        NumberiUtil.editTestFormat(etPwd,8);
      /*  etPwd.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                Log.d("???",hasFocus+"");
                if (hasFocus && !TextUtils.isEmpty(etPwd.getText()) && etPwd.getText().toString().equals("MAX")) {
                    tv_maxvote.setText("");
                }
            }
        });*/
    }

    @OnClick({R.id.tv_sure})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sure:
                //确定
                String num = etPwd.getText().toString().trim();
                if (TextUtils.isEmpty(num)) {
                    showToastMessage(getString(R.string.munnotnull));
                    return;
                }
                if (num.startsWith(".")) {
                    showToastMessage(getString(R.string.inputmustnum));
                    return;
                }
//                if (Long.parseLong(pwd) > Long.parseLong(fee)) {
//                    showToastMessage(getString(R.string.lack_of_balance));
//                    return;
//                }
                post(RxEnum.VOTETRANSFERACTIVITY.ordinal(), num, null);
                finish();

                // dialogUtil.showWarmPromptInput(this, null, null, this);

                break;

        }
    }

}
