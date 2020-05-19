package org.elastos.wallet.ela.ui.proposal.fragment;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import com.scwang.smartrefresh.layout.SmartRefreshLayout;
import com.scwang.smartrefresh.layout.api.RefreshLayout;
import com.scwang.smartrefresh.layout.listener.OnLoadMoreListener;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.proposal.adapter.ProposalRecAdapetr;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalSearchEntity;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalPresenter;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class SearchFragment extends BaseFragment implements CommonRvListener, NewBaseViewData, OnLoadMoreListener {

    @BindView(R.id.et_search)
    EditText etSearch;
    @BindView(R.id.tv_search)
    TextView tvSearch;
    @BindView(R.id.tv_nosearch)
    TextView tvNosearch;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.srl)
    SmartRefreshLayout srl;
    Unbinder unbinder;
    private ProposalPresenter presenter;
    private int pageNum = 1;
    private ArrayList<ProposalSearchEntity.DataBean.ListBean> list;
    private String searchInput;
    private ProposalRecAdapetr adapter;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_proposal_search;
    }

    @Override
    protected void initView(View view) {
        presenter = new ProposalPresenter();
        srl.setOnLoadMoreListener(this);
    }

    @OnClick({R.id.tv_search})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_search:
                //查找

                searchInput = etSearch.getText().toString();
                if (TextUtils.isEmpty(searchInput)) {
                    return;
                }
                pageNum = 1;
                presenter.proposalSearch(pageNum, 20, "ALL", searchInput, this);

                break;


        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "proposalSearch":
                setRecycleView(((ProposalSearchEntity) baseEntity).getData().getList());
                break;
        }
    }

    private void setRecycleView(List<ProposalSearchEntity.DataBean.ListBean> data) {
        if (data == null || data.size() == 0) {
            if (pageNum == 1) {
                rv.setVisibility(View.GONE);
                tvNosearch.setVisibility(View.VISIBLE);
            } else {
                showToastMessage(getString(R.string.loadall));
            }

            return;
        }

        if (list == null) {
            list = new ArrayList<>();
        }
        if (pageNum == 1) {
            rv.setVisibility(View.VISIBLE);
            tvNosearch.setVisibility(View.GONE);
            list.clear();
        }

        list.addAll(data);
        if (adapter == null) {
            adapter = new ProposalRecAdapetr(getContext(), list);
            adapter.setCommonRvListener(this);
            rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv.setAdapter(adapter);
        } else {
            adapter.notifyDataSetChanged();
        }

        pageNum++;
    }

    @Override
    public void onRvItemClick(int position, Object o) {
        Bundle bundle = new Bundle();
        ProposalSearchEntity.DataBean.ListBean bean = (ProposalSearchEntity.DataBean.ListBean) o;
        bundle.putInt("position", position);
        bundle.putParcelableArrayList("ProposalSearchDateList", list);
        start(PropasalDetailFragment.class, bundle);
    }


    @Override
    public void onLoadMore(RefreshLayout refreshLayout) {
        onErrorRefreshLayout(srl);
        presenter.proposalSearch(pageNum, 20, "ALL", searchInput, this);
    }
}
