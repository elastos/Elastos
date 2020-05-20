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
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.committee.adaper.PastCtRecAdapter;
import org.elastos.wallet.ela.ui.committee.bean.CtDetailBean;
import org.elastos.wallet.ela.ui.committee.bean.PastCtBean;
import org.elastos.wallet.ela.ui.committee.fragment.CtDismissPromptFragment;
import org.elastos.wallet.ela.ui.committee.fragment.CtListFragment;
import org.elastos.wallet.ela.ui.committee.fragment.CtManagerFragment;
import org.elastos.wallet.ela.ui.committee.fragment.SecretaryCtDetailFragment;
import org.elastos.wallet.ela.ui.committee.presenter.CtDetailPresenter;
import org.elastos.wallet.ela.ui.committee.presenter.PastCtPresenter;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.AppUtlis;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

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
    CtDetailPresenter ctDetailPresenter;
    PastCtPresenter pastCtPresenter;

    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_past_list;
    }

    @Override
    protected void initView(View view) {
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.found_ct_secretary_entrance);
        tvTitle.setText(mContext.getString(R.string.ctmemberlist));
        ctDetailPresenter = new CtDetailPresenter();
        pastCtPresenter = new PastCtPresenter();

        ctDetailPresenter.getCouncilInfo(this, "", wallet.getDid());
        pastCtPresenter.getCouncilTerm(this);

//        rockData();
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
            case "getCouncilInfo":
                go2((CtDetailBean) baseEntity);
                break;
            case "getCouncilTerm":
                setRcViewData((PastCtBean) baseEntity);
                break;
        }
    }

    private void go2(CtDetailBean ctDetailBean) {
        CtDetailBean.DataBean dataBean = ctDetailBean.getData();
        String status = dataBean.getStatus();
        String depositAmount = dataBean.getDepositAmount();
        if(AppUtlis.isNullOrEmpty(status) || status.equals("Elected")) {
//            pastCtPresenter.getCouncilTerm(this);
        } else {
            Bundle bundle = new Bundle();
            bundle.putString("status", status);
            bundle.putString("depositAmount", depositAmount);
            start(CtDismissPromptFragment.class, bundle);
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
            bean.setId(data.getId());
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
        Bundle bundle = new Bundle();
        bundle.putString("status", "Elected");
        start(CtManagerFragment.class, bundle);
    }

    @OnClick({R.id.iv_title_right})
    public void onClick(View view) {
        Bundle bundle = new Bundle();
        bundle.putString("id", "");
        bundle.putString("did", wallet.getDid());
        start(SecretaryCtDetailFragment.class, bundle);
    }
}
