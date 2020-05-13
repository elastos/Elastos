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

package org.elastos.wallet.ela.ui.Assets.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.AppCompatEditText;
import android.text.TextUtils;
import android.view.View;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.activity.PwdActivity;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class SignTransactionFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.et_sign)
    AppCompatEditText etSign;
    @BindView(R.id.et_tosign)
    AppCompatEditText etTosign;

    Unbinder unbinder;
    @BindView(R.id.rb)
    RadioGroup rb;
    private Wallet wallet;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_sign_transaction;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        wallet = data.getParcelable("wallet");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.sign);
        registReceiver();
        ((RadioButton) rb.getChildAt(0)).setChecked(true);

    }

    @OnClick({R.id.tv_copy, R.id.tv_paste, R.id.tv_tosign})
    public void onViewClicked(View view) {

        switch (view.getId()) {
            case R.id.tv_copy:
                ClipboardUtil.copyClipboar(getBaseActivity(), etSign.getText().toString().trim());
                break;
            case R.id.tv_paste:
                etTosign.setText(ClipboardUtil.paste(getBaseActivity()));
                break;
            case R.id.tv_tosign:
                RadioButton radioButton = (mRootView.findViewById(rb.getCheckedRadioButtonId()));
                if (TextUtils.isEmpty(etTosign.getText().toString().trim())) {
                    showToast("请输入代签名信息");
                    return;
                }
                Intent intent = new Intent(getActivity(), PwdActivity.class);
                intent.putExtra("wallet", wallet);
                intent.putExtra("chainId", radioButton.getText().toString().trim());
                intent.putExtra("attributes", etTosign.getText().toString().trim());
                intent.putExtra("onlySign", true);
                startActivity(intent);
                break;
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.SIGNSUCCESS.ordinal()) {
            String signData = (String) result.getObj();
            etSign.setText(signData);
            showToast("签名成功");
            ClipboardUtil.copyClipboar(getBaseActivity(), signData);
        }
    }

}
