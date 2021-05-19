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


import android.os.Bundle;
import android.support.v4.view.ViewPager;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.RadioGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.adapter.TabAdapter;

import java.util.ArrayList;

import butterknife.BindView;

/**
 * 导入钱包
 */
public class ImportWalletFragment extends BaseFragment implements RadioGroup.OnCheckedChangeListener, ViewPager.OnPageChangeListener {


    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;

    @BindView(R.id.viewpage)
    ViewPager viewpage;
    private ArrayList<BaseFragment> fragments = new ArrayList<>();
    private int[] titles = new int[]{R.string.mnemonic, R.string.keystore};
    TabAdapter pagerAdapter;
    @BindView(R.id.radiogroup)
    RadioGroup radiogroup;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_import_wallet;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.import_the_wallet));
        fragments.add(new ImportMnemonicFragment());
        fragments.add(new ImportKeystoreFragment());
        radiogroup.setOnCheckedChangeListener(this);
        radiogroup.check(R.id.radiobutton1);
        //   mTabLayout.setupWithViewPager(viewpage, false);
        pagerAdapter = new TabAdapter(getContext(), getChildFragmentManager(), fragments, titles);
        viewpage.setAdapter(pagerAdapter);
        viewpage.addOnPageChangeListener(this);
    }

    public static ImportWalletFragment newInstance() {
        Bundle args = new Bundle();
        ImportWalletFragment fragment = new ImportWalletFragment();
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCheckedChanged(RadioGroup group, int checkedId) {
        switch (checkedId) {

            case R.id.radiobutton1:
                viewpage.setCurrentItem(0);

                break;

            case R.id.radiobutton2:
                viewpage.setCurrentItem(1);
                break;
        }
    }

    @Override
    public void onPageScrolled(int i, float v, int i1) {

    }

    @Override
    public void onPageSelected(int i) {
        if (i == 0) {
            radiogroup.check(R.id.radiobutton1);
        } else if (i == 1) {
            radiogroup.check(R.id.radiobutton2);
        }
    }

    @Override
    public void onPageScrollStateChanged(int i) {

    }
}
