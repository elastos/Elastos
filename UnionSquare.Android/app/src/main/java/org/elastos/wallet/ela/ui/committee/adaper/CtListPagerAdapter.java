package org.elastos.wallet.ela.ui.committee.adaper;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;

import org.elastos.wallet.ela.base.BaseFragment;

import java.util.List;

public class CtListPagerAdapter extends FragmentPagerAdapter {

    private List<BaseFragment> fragments;

    public CtListPagerAdapter(FragmentManager manager, List<BaseFragment> fragments) {
        super(manager);
        this.fragments = fragments;
    }

    @Override
    public Fragment getItem(int i) {
        return fragments == null ? null : fragments.get(i);
    }

    @Override
    public int getCount() {
        return fragments==null ?0:fragments.size();
    }
}
