package org.elastos.wallet.ela.ui.Assets.adapter;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.PagerAdapter;
import android.view.View;

import org.elastos.wallet.ela.base.BaseFragment;

import java.util.ArrayList;
import java.util.List;

public class TabAdapter extends FragmentPagerAdapter {
    private List<BaseFragment> mFragmentList = new ArrayList<>();
    private int[] titles;
    private Context context;

    public TabAdapter(Context context, FragmentManager manager, List<BaseFragment> mFragmentList, int[] titles) {
        super(manager);
        this.mFragmentList = mFragmentList;
        this.titles = titles;
        this.context = context;
    }

    @Override
    public Fragment getItem(int position) {
        return mFragmentList.get(position);
    }

    @Override
    public int getCount() {
        return mFragmentList.size();
    }


    @Override
    public CharSequence getPageTitle(int position) {
        return context.getString(titles[position]);
    }

    public String getPageTitl(int position) {
        return context.getString(titles[position]);
    }
}
