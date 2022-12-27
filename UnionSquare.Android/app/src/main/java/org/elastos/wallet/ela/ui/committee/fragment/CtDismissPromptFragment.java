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

package org.elastos.wallet.ela.ui.committee.fragment;

import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.AppUtlis;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * dismiss prompt(expiration office, impeachment)
 */
public class CtDismissPromptFragment extends BaseFragment {

    String depositAmount;
    String status;
    String did;
    @BindView(R.id.tv_prompt)
    TextView promptTv;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_dismiss_prompt;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        did = data.getString("did");
        depositAmount = data.getString("depositAmount");
        status = data.getString("status");
    }

    @Override
    protected void initView(View view) {
        setView();
    }

    private void setView() {

        switch (status) {
            case "Terminated":
                promptTv.setText(getString(R.string.completedialoghint));
                break;
            case "Impeached":
                promptTv.setText(getString(R.string.canceldialoghint));
                break;
            case "Returned":
                promptTv.setText(getString(R.string.dimissdialoghint));
                break;
            default:
        }
    }

    @OnClick({R.id.tv_close, R.id.tv_deposit})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.tv_close:
                popBackFragment();
                break;
            case R.id.tv_deposit:
                Bundle bundle = new Bundle();
                bundle.putString("did", did);
                bundle.putString("status", status);
                bundle.putString("depositAmount", AppUtlis.isNullOrEmpty(depositAmount)?"0":depositAmount);
                start(CtManagerFragment.class, bundle);
                break;
        }
    }
}
