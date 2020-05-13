package org.elastos.wallet.ela.ui.committee.fragment;

import android.support.v4.view.ViewPager;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.committee.adaper.CtListPagerAdapter;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * Committee and secretary general list container
 */
public class CtListFragment extends BaseFragment {
    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.vp)
    ViewPager viewpager;
    @BindView(R.id.ct_tv)
    TextView ctTitleTv;
    @BindView(R.id.line1)
    View line1;
    @BindView(R.id.secretary_tv)
    TextView secretaryTitleTv;
    @BindView(R.id.line2)
    View line2;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_list;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.ctlisttitle));

        ArrayList<BaseFragment> datas = new ArrayList<BaseFragment>();
        datas.add(GeneralCtListFragment.getInstance(0));
        datas.add(SecretaryCtListFragment.getInstance(1));

        CtListPagerAdapter mPagerAdapter = new CtListPagerAdapter(getFragmentManager(), datas);
        viewpager.setAdapter(mPagerAdapter);
        viewpager.addOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener(){
            @Override
            public void onPageSelected(int position) {
                super.onPageSelected(position);
                if(0 == position) {
                    selectCtList();
                } else if(1 == position) {
                    selectSecretaryList();
                }
            }
        });
    }


    private void selectCtList() {
        viewpager.setCurrentItem(0);
        line1.setVisibility(View.VISIBLE);
        line2.setVisibility(View.GONE);
        ctTitleTv.setTextColor(getResources().getColor(R.color.whiter));
        secretaryTitleTv.setTextColor(getResources().getColor(R.color.whiter50));
    }

    private void selectSecretaryList() {
        viewpager.setCurrentItem(1);
        line1.setVisibility(View.GONE);
        line2.setVisibility(View.VISIBLE);
        ctTitleTv.setTextColor(getResources().getColor(R.color.whiter50));
        secretaryTitleTv.setTextColor(getResources().getColor(R.color.whiter));
    }

    @OnClick({R.id.ct_layout, R.id.secretary_layout})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.ct_layout:
                selectCtList();
                break;
            case R.id.secretary_layout:
                //TODO daokun.xi test
//                selectSecretaryList();
//                start(GeneralCtDetailFragment.class);
                start(CtManagerFragment.class);
                break;
        }
    }
}
