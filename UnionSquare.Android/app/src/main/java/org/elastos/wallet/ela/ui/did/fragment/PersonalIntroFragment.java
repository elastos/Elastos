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
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;

public class PersonalIntroFragment extends BaseFragment {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_progress)
    TextView tvProgress;
    @BindView(R.id.et_intro)
    EditText etIntro;
    @BindView(R.id.tv_next)
    TextView tvNext;
    private String personalIntro;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_personalintro;
    }

    @Override
    protected void setExtraData(Bundle data) {
        personalIntro = data.getString("personalIntro", "");
        tvProgress.setText(personalIntro.length() + "/800");

    }


    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.personalintro));
        if (!TextUtils.isEmpty(personalIntro)) {
            etIntro.setText(personalIntro);
        }
        etIntro.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

            }

            @Override
            public void afterTextChanged(Editable s) {
                tvProgress.setText(s.length() + "/800");
            }
        });
    }

    @OnClick({R.id.tv_next})
    public void onViewClicked(View view) {
        switch (view.getId()) {

            case R.id.tv_next:
                popBackFragment();
                break;


        }
    }

    @Override
    public void onDestroy() {
        post(RxEnum.EDITPERSONALINTRO.ordinal(), null, getText(etIntro));
        super.onDestroy();
    }
}
