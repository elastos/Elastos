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

package org.elastos.wallet.ela.ui.Assets;

import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.AddressListFragment;
import org.elastos.wallet.ela.utils.ClipboardUtil;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class ErrorScanFragment extends BaseFragment {
    @BindView(R.id.tv_erro)
    TextView tvErro;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @Override
    protected void setExtraData(Bundle data) {
        String result = data.getString("result");
        tvErro.setText(result);
    }

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_errorscan;
    }

    @Override
    protected void initView(View view) {
        tvErro.setScrollbarFadingEnabled(false);
        tvErro.setMovementMethod(ScrollingMovementMethod.getInstance());
    }

    @OnClick({R.id.tv_copy})
    public void onViewClicked(View view) {
        //复制
        ClipboardUtil.copyClipboar(getBaseActivity(), tvErro.getText().toString().trim());
    }
}
