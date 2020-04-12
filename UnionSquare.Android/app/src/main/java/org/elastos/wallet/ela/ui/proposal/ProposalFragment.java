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

package org.elastos.wallet.ela.ui.proposal;

import android.support.design.widget.TabLayout;
import android.support.v4.view.ViewPager;
import android.util.TypedValue;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.proposal.adapter.ProposalPagerAdapter;
import org.elastos.wallet.ela.utils.Log;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.Unbinder;

public class ProposalFragment extends BaseFragment implements TabLayout.BaseOnTabSelectedListener {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.tab)
    TabLayout tab;
    @BindView(R.id.vp)
    ViewPager vp;
    Unbinder unbinder;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_proposal;
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.findlistup2);
        List<String> titles = new ArrayList<String>();

        titles.add(getString(R.string.total));
        titles.add(getString(R.string.menberreview));
        titles.add(getString(R.string.publishing));
        titles.add(getString(R.string.executing));
        titles.add(getString(R.string.finish));
        titles.add(getString(R.string.abolished));
        //设置适配器
        ProposalPagerAdapter mPagerAdapter = new ProposalPagerAdapter(titles, getContext());
        vp.setAdapter(mPagerAdapter);

        //关联
        tab.setupWithViewPager(vp);
        //vp.setOffscreenPageLimit(3);
        tab.addOnTabSelectedListener(this);

    }


    @Override
    public void onTabSelected(TabLayout.Tab tab) {
        TextView textView = new TextView(getActivity());
        textView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 13);
        textView.setText(tab.getText());
        tab.setCustomView(textView);
        Log.i("????", tab.getPosition());
        switch (tab.getPosition()) {
            case 0:

                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
        }

    }

    @Override
    public void onTabUnselected(TabLayout.Tab tab) {
        tab.setCustomView(null);
    }

    @Override
    public void onTabReselected(TabLayout.Tab tab) {
        Log.i("????11", tab.getPosition());
    }
}
