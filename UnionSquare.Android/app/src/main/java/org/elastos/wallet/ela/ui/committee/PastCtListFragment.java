package org.elastos.wallet.ela.ui.committee;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.scwang.smartrefresh.layout.api.RefreshLayout;
import com.scwang.smartrefresh.layout.listener.OnLoadMoreListener;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.committee.adaper.PastCtRecAdapter;
import org.elastos.wallet.ela.ui.committee.bean.PastCtBean;
import org.elastos.wallet.ela.ui.committee.fragment.CtListFragment;
import org.elastos.wallet.ela.ui.committee.fragment.CtManagerFragment;
import org.elastos.wallet.ela.ui.committee.presenter.PastCtPresenter;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;

/**
 * List of past members
 */
public class PastCtListFragment extends BaseFragment implements NewBaseViewData, CommonRvListener, PastCtRecAdapter.ManagerListener, OnLoadMoreListener {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.recyclerview)
    RecyclerView recyclerview;
    PastCtRecAdapter adapter;
    List<PastCtBean> list;
    PastCtPresenter presenter;
    private int pageNum = 1;
    private final int pageSize = 1000;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_past_list;
    }

    @Override
    protected void initView(View view) {
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.found_ct_secretary_entrance);
        tvTitle.setText(mContext.getString(R.string.pastcttitle));
        presenter = new PastCtPresenter();
        presenter.getPastCtList(pageNum, pageSize, "all", this, true);

        //rock data
        list = new ArrayList<>();
        PastCtBean pastCtBean1 = new PastCtBean();
        pastCtBean1.setIndex("1");
        pastCtBean1.setType(1);
        pastCtBean1.setTime("2019.03.01-2019.04.01");
        list.add(pastCtBean1);

        PastCtBean pastCtBean2 = new PastCtBean();
        pastCtBean2.setIndex("2");
        pastCtBean2.setType(2);
        pastCtBean2.setTime("2019.03.01-2019.04.01");
        list.add(pastCtBean2);

        PastCtBean pastCtBean3 = new PastCtBean();
        pastCtBean3.setIndex("3");
        pastCtBean3.setType(3);
        pastCtBean3.setTime("2019.03.01-2019.04.01");
        list.add(pastCtBean3);

        PastCtBean pastCtBean4 = new PastCtBean();
        pastCtBean4.setIndex("4");
        pastCtBean4.setTime("2019.03.01-2019.04.01");
        list.add(pastCtBean4);

        PastCtBean pastCtBean5 = new PastCtBean();
        pastCtBean5.setIndex("5");
        pastCtBean5.setTime("2019.03.01-2019.04.01");
        list.add(pastCtBean5);

        PastCtBean pastCtBean6 = new PastCtBean();
        pastCtBean6.setIndex("6");
        pastCtBean6.setTime("2019.03.01-2019.04.01");
        list.add(pastCtBean6);

        PastCtBean pastCtBean7 = new PastCtBean();
        pastCtBean7.setIndex("7");
        pastCtBean7.setTime("2019.03.01-2019.04.01");
        list.add(pastCtBean7);


        PastCtBean pastCtBean8 = new PastCtBean();
        pastCtBean8.setIndex("8");
        pastCtBean8.setTime("2019.03.01-2019.04.01");
        list.add(pastCtBean8);

        PastCtBean pastCtBean9 = new PastCtBean();
        pastCtBean9.setIndex("9");
        pastCtBean9.setTime("2019.03.01-2019.04.01");
        list.add(pastCtBean9);

        PastCtBean pastCtBean10 = new PastCtBean();
        pastCtBean10.setIndex("10");
        pastCtBean10.setTime("2019.03.01-2019.04.01");
        list.add(pastCtBean10);

        setRecycleView();
    }

    private void setRecycleView() {
        if (adapter == null) {
            adapter = new PastCtRecAdapter(getContext(), list);
            recyclerview.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            recyclerview.setAdapter(adapter);
            adapter.setCommonRvListener(this);
            adapter.setManagerListener(this);
        } else {
            adapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "":
                list = new ArrayList<>();
                setRecycleView();
                break;
        }
    }

    @Override
    public void onRvItemClick(int position, Object o) {
        Bundle bundle = new Bundle();
        bundle.putString("index", list.get(position).getIndex());
        start(CtListFragment.class, bundle);
    }

    @Override
    public void onLoadMore(RefreshLayout refreshLayout) {

    }

    @Override
    public void onManagerClick(int position) {
        start(CtManagerFragment.class);
    }
}
