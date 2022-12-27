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

package org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet;

import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.ImportKeystoreFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.WalletListFragment;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class MainPrivateKeyFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_mainprivatekey;
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.addmainkey));
    }

    @OnClick({R.id.rl_new, R.id.rl_import, R.id.rl_use})
    public void onViewClick(View view) {
        switch (view.getId()) {
            case R.id.rl_new:
                start(CreatePrivateKey.class);
                break;
            case R.id.rl_import:
                start(ImportMnemonicPageFragment.class);
                break;
            case R.id.rl_use:
                Bundle bundle=new Bundle();
                bundle.putString("openType","showList");
                start(WalletListFragment.class,bundle);
                break;

        }
    }


}
