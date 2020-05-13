package org.elastos.wallet.ela.ui.committee.fragment;

import android.graphics.Color;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.committee.adaper.GeneralCtRecAdapter;
import org.elastos.wallet.ela.ui.committee.bean.GeneralCtBean;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;

public class GeneralCtListFragment extends BaseFragment implements CommonRvListener, NewBaseViewData {
    @BindView(R.id.rv)
    RecyclerView recyclerview;
    GeneralCtRecAdapter adapter;
    List<GeneralCtBean> list;


    private void setRecycleView() {
        if (adapter == null) {
            adapter = new GeneralCtRecAdapter(getContext(), list);
            recyclerview.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            recyclerview.setAdapter(adapter);
            adapter.setCommonRvListener(this);

        } else {
            adapter.notifyDataSetChanged();
        }
    }


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_general_list;
    }

    @Override
    protected void initView(View view) {
        mRootView.setBackgroundColor(Color.parseColor("#00000000"));

        list = new ArrayList<>();
        GeneralCtBean GeneralCtBean1 = new GeneralCtBean();
        GeneralCtBean1.setName("Feng Zhang1");
        GeneralCtBean1.setLocation("中国");
        list.add(GeneralCtBean1);

        GeneralCtBean GeneralCtBean2 = new GeneralCtBean();
        GeneralCtBean2.setName("Feng Zhang2");
        GeneralCtBean2.setLocation("中国");
        list.add(GeneralCtBean2);

        GeneralCtBean GeneralCtBean3 = new GeneralCtBean();
        GeneralCtBean3.setName("Feng Zhang3");
        GeneralCtBean3.setLocation("中国");
        list.add(GeneralCtBean3);

        GeneralCtBean GeneralCtBean4 = new GeneralCtBean();
        GeneralCtBean4.setName("Feng Zhang4");
        GeneralCtBean4.setLocation("中国");
        list.add(GeneralCtBean4);

        GeneralCtBean GeneralCtBean5 = new GeneralCtBean();
        GeneralCtBean5.setName("Feng Zhang5");
        GeneralCtBean5.setLocation("中国");
        list.add(GeneralCtBean5);

        GeneralCtBean GeneralCtBean6 = new GeneralCtBean();
        GeneralCtBean6.setName("Feng Zhang6");
        GeneralCtBean6.setLocation("中国");
        list.add(GeneralCtBean6);

        GeneralCtBean GeneralCtBean7 = new GeneralCtBean();
        GeneralCtBean7.setName("Feng Zhang7");
        GeneralCtBean7.setLocation("中国");
        list.add(GeneralCtBean7);


        GeneralCtBean GeneralCtBean8 = new GeneralCtBean();
        GeneralCtBean8.setName("Feng Zhang8");
        GeneralCtBean8.setLocation("中国");
        list.add(GeneralCtBean8);

        GeneralCtBean GeneralCtBean9 = new GeneralCtBean();
        GeneralCtBean9.setName("Feng Zhang9");
        GeneralCtBean9.setLocation("中国");
        list.add(GeneralCtBean9);

        GeneralCtBean GeneralCtBean10 = new GeneralCtBean();
        GeneralCtBean10.setName("Feng Zhang10");
        GeneralCtBean10.setLocation("中国");
        list.add(GeneralCtBean10);

        setRecycleView();
    }


    public static GeneralCtListFragment getInstance(int tag) {
        GeneralCtListFragment generalCtListFragment = new GeneralCtListFragment();
        Bundle bundle = new Bundle();
        bundle.putInt("TAG", tag);
        generalCtListFragment.setExtraData(bundle);
        return generalCtListFragment;
    }

    @Override
    public void onRvItemClick(int position, Object o) {
        Bundle bundle = new Bundle();
        bundle.putString("cid", "123");
        start(GeneralCtDetailFragment.class, bundle);
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

    }
}
