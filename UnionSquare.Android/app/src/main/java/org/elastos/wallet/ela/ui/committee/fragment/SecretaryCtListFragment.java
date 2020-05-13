package org.elastos.wallet.ela.ui.committee.fragment;

import android.graphics.Color;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.committee.adaper.SecretaryCtRecAdapter;
import org.elastos.wallet.ela.ui.committee.bean.SecretaryCtBean;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;

public class SecretaryCtListFragment extends BaseFragment implements CommonRvListener {
    @BindView(R.id.rv)
    RecyclerView recyclerview;
    SecretaryCtRecAdapter adapter;
    List<SecretaryCtBean> list;


    private void setRecycleView() {
        if (adapter == null) {
            adapter = new SecretaryCtRecAdapter(getContext(), list);
            recyclerview.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            recyclerview.setAdapter(adapter);
            adapter.setCommonRvListener(this);

        } else {
            adapter.notifyDataSetChanged();
        }
    }

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_secretary_list;
    }

    @Override
    protected void initView(View view) {
        mRootView.setBackgroundColor(Color.parseColor("#00000000"));

        list = new ArrayList<>();
        SecretaryCtBean SecretaryCtBean1 = new SecretaryCtBean();
        SecretaryCtBean1.setName("Feng Zhang1");
        SecretaryCtBean1.setLocation("中国");
        list.add(SecretaryCtBean1);

        SecretaryCtBean SecretaryCtBean2 = new SecretaryCtBean();
        SecretaryCtBean2.setName("Feng Zhang2");
        SecretaryCtBean2.setLocation("中国");
        list.add(SecretaryCtBean2);

        SecretaryCtBean SecretaryCtBean3 = new SecretaryCtBean();
        SecretaryCtBean3.setName("Feng Zhang3");
        SecretaryCtBean3.setLocation("中国");
        list.add(SecretaryCtBean3);

        SecretaryCtBean SecretaryCtBean4 = new SecretaryCtBean();
        SecretaryCtBean4.setName("Feng Zhang4");
        SecretaryCtBean4.setLocation("中国");
        list.add(SecretaryCtBean4);

        SecretaryCtBean SecretaryCtBean5 = new SecretaryCtBean();
        SecretaryCtBean5.setName("Feng Zhang5");
        SecretaryCtBean5.setLocation("中国");
        list.add(SecretaryCtBean5);

        SecretaryCtBean SecretaryCtBean6 = new SecretaryCtBean();
        SecretaryCtBean6.setName("Feng Zhang6");
        SecretaryCtBean6.setLocation("中国");
        list.add(SecretaryCtBean6);

        SecretaryCtBean SecretaryCtBean7 = new SecretaryCtBean();
        SecretaryCtBean7.setName("Feng Zhang7");
        SecretaryCtBean7.setLocation("中国");
        list.add(SecretaryCtBean7);


        SecretaryCtBean SecretaryCtBean8 = new SecretaryCtBean();
        SecretaryCtBean8.setName("Feng Zhang8");
        SecretaryCtBean8.setLocation("中国");
        list.add(SecretaryCtBean8);

        SecretaryCtBean SecretaryCtBean9 = new SecretaryCtBean();
        SecretaryCtBean9.setName("Feng Zhang9");
        SecretaryCtBean9.setLocation("中国");
        list.add(SecretaryCtBean9);

        SecretaryCtBean SecretaryCtBean10 = new SecretaryCtBean();
        SecretaryCtBean10.setName("Feng Zhang10");
        SecretaryCtBean10.setLocation("中国");
        list.add(SecretaryCtBean10);

        setRecycleView();
    }

    public static SecretaryCtListFragment getInstance(int tag) {
        SecretaryCtListFragment secretaryCtListFragment = new SecretaryCtListFragment();
        Bundle bundle = new Bundle();
        bundle.putInt("TAG", tag);
        secretaryCtListFragment.setExtraData(bundle);
        return secretaryCtListFragment;
    }

    @Override
    public void onRvItemClick(int position, Object o) {

    }
}
