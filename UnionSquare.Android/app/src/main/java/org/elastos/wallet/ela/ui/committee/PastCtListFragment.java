package org.elastos.wallet.ela.ui.committee;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
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
import org.elastos.wallet.ela.utils.AppUtlis;

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
    List<PastCtBean.DataBean> list;
    PastCtPresenter presenter;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_past_list;
    }

    @Override
    protected void initView(View view) {
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.found_ct_secretary_entrance);
        tvTitle.setText(mContext.getString(R.string.ctmemberlist));
        presenter = new PastCtPresenter();
        presenter.getCouncilTerm(this);

        rockData();
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
        if(!AppUtlis.isNullOrEmpty(methodName) && methodName.equals("getCouncilTerm")) {
            setRcViewData((PastCtBean) baseEntity);
        }
    }

    private void setRcViewData(PastCtBean pastCtBean) {
        if(null == pastCtBean) return;
        List<PastCtBean.DataBean> datas = pastCtBean.getData();
        if(datas==null || datas.size()<=0) return;
        if(null == list) {
            list = new ArrayList<>();
        } else {
            list.clear();
        }
        for(PastCtBean.DataBean data : datas) {
            PastCtBean.DataBean bean = new PastCtBean.DataBean();
            bean.setIndex(data.getIndex());
            bean.setStatus(data.getStatus());
            bean.setStartDate(data.getStartDate());
            bean.setEndDate(data.getEndDate());
            list.add(bean);
        }
        setRecycleView();
    }

    @Override
    public void onRvItemClick(int position, Object o) {
        Bundle bundle = new Bundle();
        bundle.putInt("index", list.get(position).getIndex());
        start(CtListFragment.class, bundle);
    }

    @Override
    public void onLoadMore(RefreshLayout refreshLayout) {

    }

    @Override
    public void onManagerClick(int position) {
        start(CtManagerFragment.class);
    }

    private void rockData() {
        //rock data
        list = new ArrayList<>();
        PastCtBean.DataBean pastCtBean1 = new PastCtBean.DataBean();
        pastCtBean1.setIndex(1);
        pastCtBean1.setStatus("HISTORY");
        pastCtBean1.setStartDate("1589271912");
        pastCtBean1.setEndDate("1589272000");

        PastCtBean.DataBean pastCtBean2 = new PastCtBean.DataBean();
        pastCtBean2.setIndex(2);
        pastCtBean2.setStatus("HISTORY");
        pastCtBean2.setStartDate("1589271912");
        pastCtBean2.setEndDate("1589272000");

        PastCtBean.DataBean pastCtBean3 = new PastCtBean.DataBean();
        pastCtBean3.setIndex(3);
        pastCtBean3.setStatus("HISTORY");
        pastCtBean3.setStartDate("1589271912");
        pastCtBean3.setEndDate("1589272000");

        PastCtBean.DataBean pastCtBean4 = new PastCtBean.DataBean();
        pastCtBean4.setIndex(4);
        pastCtBean4.setStatus("HISTORY");
        pastCtBean4.setStartDate("1589271912");
        pastCtBean4.setEndDate("1589272000");

        PastCtBean.DataBean pastCtBean5 = new PastCtBean.DataBean();
        pastCtBean5.setIndex(5);
        pastCtBean5.setStatus("HISTORY");
        pastCtBean5.setStartDate("1589271912");
        pastCtBean5.setEndDate("1589272000");

        PastCtBean.DataBean pastCtBean6 = new PastCtBean.DataBean();
        pastCtBean6.setIndex(6);
        pastCtBean6.setStatus("HISTORY");
        pastCtBean6.setStartDate("1589271912");
        pastCtBean6.setEndDate("1589272000");

        PastCtBean.DataBean pastCtBean7 = new PastCtBean.DataBean();
        pastCtBean7.setIndex(7);
        pastCtBean7.setStatus("HISTORY");
        pastCtBean7.setStartDate("1589271912");
        pastCtBean7.setEndDate("1589272000");

        PastCtBean.DataBean pastCtBean8 = new PastCtBean.DataBean();
        pastCtBean8.setIndex(8);
        pastCtBean8.setStatus("HISTORY");
        pastCtBean8.setStartDate("1589271912");
        pastCtBean8.setEndDate("1589272000");

        PastCtBean.DataBean pastCtBean9 = new PastCtBean.DataBean();
        pastCtBean9.setIndex(9);
        pastCtBean9.setStatus("CURRENT");
        pastCtBean9.setStartDate("1589271912");
        pastCtBean9.setEndDate("1589272000");

        PastCtBean.DataBean pastCtBean10 = new PastCtBean.DataBean();
        pastCtBean10.setIndex(10);
        pastCtBean10.setStatus("VOTING");
        pastCtBean10.setStartDate("1589271912");
        pastCtBean10.setEndDate("1589272000");

        list.add(pastCtBean10);
        list.add(pastCtBean9);
        list.add(pastCtBean8);
        list.add(pastCtBean7);
        list.add(pastCtBean6);
        list.add(pastCtBean5);
        list.add(pastCtBean4);
        list.add(pastCtBean3);
        list.add(pastCtBean2);
        list.add(pastCtBean1);

        setRecycleView();
    }
}
