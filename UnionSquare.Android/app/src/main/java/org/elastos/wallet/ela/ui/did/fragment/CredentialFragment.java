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

package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.OnClick;

public class CredentialFragment extends BaseFragment {


    @BindView(R.id.tv_personlinfo_time)
    TextView tvPersonlinfoTime;
    @BindView(R.id.tv_personlinfo_no)
    TextView tvPersonlinfoNo;

    @BindView(R.id.tv_title)
    TextView tvTitle;

    private CredentialSubjectBean credentialSubjectBean;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_did_credential;
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.credentialinfo);
        viewInfo();
        registReceiver();

    }

    private void viewInfo() {
        String json = getMyDID().getCredentialProFromStore(getMyDID().getDidString());
        credentialSubjectBean = JSON.parseObject(json, CredentialSubjectBean.class);
        Log.i("??", json + "\n" + (JSON.parseObject("{}", CredentialSubjectBean.class) == null));//false
        if (credentialSubjectBean == null || credentialSubjectBean.whetherEmpty()) {
            tvPersonlinfoNo.setVisibility(View.VISIBLE);
            tvPersonlinfoTime.setVisibility(View.GONE);

        } else {
            tvPersonlinfoNo.setVisibility(View.GONE);
            tvPersonlinfoTime.setVisibility(View.VISIBLE);
            tvPersonlinfoTime.setText(getString(R.string.keeptime) + DateUtil.time(credentialSubjectBean.getEditTime(), getContext()));

        }
    }


    @OnClick({R.id.ll_personalinfo, R.id.tv_out, R.id.tv_in})
    public void onViewClicked(View view) {
        Bundle bundle = getArguments();

        switch (view.getId()) {
            case R.id.ll_personalinfo:
                if (credentialSubjectBean == null || credentialSubjectBean.whetherEmpty()) {
                    bundle.putString("type", Constant.EDITCREDENTIAL);
                    start(AddPersonalInfoFragment.class, bundle);
                } else {
                    bundle.putParcelable("credentialSubjectBean", credentialSubjectBean);
                    start(ShowPersonalInfoFragemnt.class, bundle);
                }
                break;

            case R.id.tv_out:
                if (credentialSubjectBean == null || credentialSubjectBean.whetherEmpty()) {
                    showToast(getString(R.string.credencialisnull));
                    break;
                }
                start(ExportCredencialFragment.class, getArguments());
                break;
            case R.id.tv_in:
                start(ImportCredencialFragment.class, getArguments());
                break;
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.EDITPERSONALINFO.ordinal()) {
            viewInfo();

        }


    }
}
