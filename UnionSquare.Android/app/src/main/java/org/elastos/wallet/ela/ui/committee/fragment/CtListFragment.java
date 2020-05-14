package org.elastos.wallet.ela.ui.committee.fragment;

import android.support.v4.view.ViewPager;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.committee.adaper.CtListPagerAdapter;
import org.elastos.wallet.ela.ui.committee.adaper.GeneralCtRecAdapter;
import org.elastos.wallet.ela.ui.committee.adaper.SecretaryCtRecAdapter;
import org.elastos.wallet.ela.ui.committee.bean.GeneralCtBean;
import org.elastos.wallet.ela.ui.committee.bean.SecretaryCtBean;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;

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
    @BindView(R.id.ct_tv)
    TextView ctTitleTv;
    @BindView(R.id.line1)
    View line1;
    @BindView(R.id.secretary_tv)
    TextView secretaryTitleTv;
    @BindView(R.id.line2)
    View line2;
    @BindView(R.id.ct_general_layout)
    View generalLayout;
    @BindView(R.id.ct_secretary_layout)
    View secretaryLayout;

    @BindView(R.id.general_rv)
    RecyclerView generalRv;
    GeneralCtRecAdapter generalAdapter;
    List<GeneralCtBean> generalList;

    @BindView(R.id.secretary_rv)
    RecyclerView secretaryRv;
    SecretaryCtRecAdapter secretaryAdapter;
    List<SecretaryCtBean> secretaryList;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_list;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.ctlisttitle));

        generalRockData();
        secretaryRockData();

        selectCtList();

    }


    private void selectCtList() {
        line1.setVisibility(View.VISIBLE);
        line2.setVisibility(View.GONE);
        ctTitleTv.setTextColor(getResources().getColor(R.color.whiter));
        secretaryTitleTv.setTextColor(getResources().getColor(R.color.whiter50));
        generalLayout.setVisibility(View.VISIBLE);
        secretaryLayout.setVisibility(View.GONE);
        setGeneralRv();
    }

    private void selectSecretaryList() {
        line1.setVisibility(View.GONE);
        line2.setVisibility(View.VISIBLE);
        ctTitleTv.setTextColor(getResources().getColor(R.color.whiter50));
        secretaryTitleTv.setTextColor(getResources().getColor(R.color.whiter));
        generalLayout.setVisibility(View.GONE);
        secretaryLayout.setVisibility(View.VISIBLE);
        setSecretaryRv();
    }

    private void setGeneralRv() {
        if (generalAdapter == null) {
            generalAdapter = new GeneralCtRecAdapter(getContext(), generalList);
            generalRv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            generalRv.setAdapter(generalAdapter);
            generalAdapter.setCommonRvListener((position, o) -> {
                start(GeneralCtDetailFragment.class);
            });

        } else {
            generalAdapter.notifyDataSetChanged();
        }
    }

    private void setSecretaryRv() {
        if (secretaryAdapter == null) {
            secretaryAdapter = new SecretaryCtRecAdapter(getContext(), secretaryList);
            secretaryRv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            secretaryRv.setAdapter(secretaryAdapter);
            secretaryAdapter.setCommonRvListener((position, o) -> {
                start(SecretaryCtDetailFragment.class);
            });

        } else {
            secretaryAdapter.notifyDataSetChanged();
        }
    }

    private void secretaryRockData() {
        secretaryList = new ArrayList<>();
        SecretaryCtBean SecretaryCtBean1 = new SecretaryCtBean();
        SecretaryCtBean1.setName("Feng Zhang1");
        SecretaryCtBean1.setLocation("中国");
        secretaryList.add(SecretaryCtBean1);

        SecretaryCtBean SecretaryCtBean2 = new SecretaryCtBean();
        SecretaryCtBean2.setName("Feng Zhang2");
        SecretaryCtBean2.setLocation("中国");
        secretaryList.add(SecretaryCtBean2);

        SecretaryCtBean SecretaryCtBean3 = new SecretaryCtBean();
        SecretaryCtBean3.setName("Feng Zhang3");
        SecretaryCtBean3.setLocation("中国");
        secretaryList.add(SecretaryCtBean3);

        SecretaryCtBean SecretaryCtBean4 = new SecretaryCtBean();
        SecretaryCtBean4.setName("Feng Zhang4");
        SecretaryCtBean4.setLocation("中国");
        secretaryList.add(SecretaryCtBean4);

        SecretaryCtBean SecretaryCtBean5 = new SecretaryCtBean();
        SecretaryCtBean5.setName("Feng Zhang5");
        SecretaryCtBean5.setLocation("中国");
        secretaryList.add(SecretaryCtBean5);

        SecretaryCtBean SecretaryCtBean6 = new SecretaryCtBean();
        SecretaryCtBean6.setName("Feng Zhang6");
        SecretaryCtBean6.setLocation("中国");
        secretaryList.add(SecretaryCtBean6);

        SecretaryCtBean SecretaryCtBean7 = new SecretaryCtBean();
        SecretaryCtBean7.setName("Feng Zhang7");
        SecretaryCtBean7.setLocation("中国");
        secretaryList.add(SecretaryCtBean7);


        SecretaryCtBean SecretaryCtBean8 = new SecretaryCtBean();
        SecretaryCtBean8.setName("Feng Zhang8");
        SecretaryCtBean8.setLocation("中国");
        secretaryList.add(SecretaryCtBean8);

        SecretaryCtBean SecretaryCtBean9 = new SecretaryCtBean();
        SecretaryCtBean9.setName("Feng Zhang9");
        SecretaryCtBean9.setLocation("中国");
        secretaryList.add(SecretaryCtBean9);

        SecretaryCtBean SecretaryCtBean10 = new SecretaryCtBean();
        SecretaryCtBean10.setName("Feng Zhang10");
        SecretaryCtBean10.setLocation("中国");
        secretaryList.add(SecretaryCtBean10);
    }

    private void generalRockData() {
        generalList = new ArrayList<>();
        GeneralCtBean GeneralCtBean1 = new GeneralCtBean();
        GeneralCtBean1.setName("Feng1");
        GeneralCtBean1.setLocation("中国");
        generalList.add(GeneralCtBean1);

        GeneralCtBean GeneralCtBean2 = new GeneralCtBean();
        GeneralCtBean2.setName("Feng2");
        GeneralCtBean2.setLocation("中国");
        generalList.add(GeneralCtBean2);

        GeneralCtBean GeneralCtBean3 = new GeneralCtBean();
        GeneralCtBean3.setName("Feng3");
        GeneralCtBean3.setLocation("中国");
        generalList.add(GeneralCtBean3);

        GeneralCtBean GeneralCtBean4 = new GeneralCtBean();
        GeneralCtBean4.setName("Feng4");
        GeneralCtBean4.setLocation("中国");
        generalList.add(GeneralCtBean4);

        GeneralCtBean GeneralCtBean5 = new GeneralCtBean();
        GeneralCtBean5.setName("Feng5");
        GeneralCtBean5.setLocation("中国");
        generalList.add(GeneralCtBean5);

        GeneralCtBean GeneralCtBean6 = new GeneralCtBean();
        GeneralCtBean6.setName("Feng6");
        GeneralCtBean6.setLocation("中国");
        generalList.add(GeneralCtBean6);

        GeneralCtBean GeneralCtBean7 = new GeneralCtBean();
        GeneralCtBean7.setName("Feng7");
        GeneralCtBean7.setLocation("中国");
        generalList.add(GeneralCtBean7);


        GeneralCtBean GeneralCtBean8 = new GeneralCtBean();
        GeneralCtBean8.setName("Feng8");
        GeneralCtBean8.setLocation("中国");
        generalList.add(GeneralCtBean8);

        GeneralCtBean GeneralCtBean9 = new GeneralCtBean();
        GeneralCtBean9.setName("Feng Zhang9");
        GeneralCtBean9.setLocation("中国");
        generalList.add(GeneralCtBean9);

        GeneralCtBean GeneralCtBean10 = new GeneralCtBean();
        GeneralCtBean10.setName("Feng Zhang10");
        GeneralCtBean10.setLocation("中国");
        generalList.add(GeneralCtBean10);
    }

    @OnClick({R.id.ct_layout, R.id.secretary_layout})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.ct_layout:
                selectCtList();
                break;
            case R.id.secretary_layout:
                selectSecretaryList();
                break;
        }
    }
}
