package org.elastos.wallet.ela.ui.committee;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.scwang.smartrefresh.layout.api.RefreshLayout;
import com.scwang.smartrefresh.layout.listener.OnLoadMoreListener;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.committee.adaper.PastCtRecAdapter;
import org.elastos.wallet.ela.ui.committee.bean.PastCtBean;
import org.elastos.wallet.ela.ui.committee.fragment.CtListFragment;
import org.elastos.wallet.ela.ui.committee.fragment.CtManagerFragment;
import org.elastos.wallet.ela.ui.committee.fragment.SecretaryCtDetailFragment;
import org.elastos.wallet.ela.ui.committee.presenter.CtManagePresenter;
import org.elastos.wallet.ela.ui.committee.presenter.PastCtPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.bean.CrStatusBean;
import org.elastos.wallet.ela.ui.crvote.fragment.CRManageFragment;
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
    PastCtPresenter pastCtPresenter;
    CtManagePresenter ctManagePresenter;
    private boolean isCrc = false;
    private boolean isVoting = false;

    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_past_list;
    }

    String did;
    String type;
    String status;
    String cid;
    String depositAmount;

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        did = data.getString("did");
        type = data.getString("type");
        status = data.getString("status");
        cid = data.getString("cid");
        depositAmount = data.getString("depositAmount");

        electedStatus();
    }

    @Override
    protected void initView(View view) {
        ivTitleRight.setImageResource(R.mipmap.found_ct_secretary_entrance);
        tvTitle.setText(mContext.getString(R.string.ctmemberlist));
        pastCtPresenter = new PastCtPresenter();
        ctManagePresenter = new CtManagePresenter();
        if (!AppUtlis.isNullOrEmpty(type) && type.equalsIgnoreCase("SecretaryGeneral")) {
            ivTitleRight.setVisibility(View.VISIBLE);
        } else if (!AppUtlis.isNullOrEmpty(type)
                && type.equalsIgnoreCase("CouncilMember")
                && !AppUtlis.isNullOrEmpty(depositAmount)
                && !depositAmount.trim().equalsIgnoreCase("0")) {
            isCrc = true;
        } else {
            isCrc = false;
            ivTitleRight.setVisibility(View.GONE);
        }
        pastCtPresenter.getCouncilTerm(this);
    }

    private void setRecycleView() {
        if (adapter == null) {
            adapter = new PastCtRecAdapter(getContext(), list, isCrc, isVoting);
            recyclerview.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            recyclerview.setAdapter(adapter);
            adapter.setCommonRvListener(this);
            adapter.setManagerListener(this);
        } else {
            adapter.notifyDataSetChanged();
        }
    }

    CrStatusBean crStatusBean = null;

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getCouncilTerm":
                setRcViewData((PastCtBean) baseEntity);
                break;
            case "getRegisteredCRInfo":
                crStatusBean = JSON.parseObject(((CommmonStringEntity) baseEntity).getData(), CrStatusBean.class);
                ctManagePresenter.getCRlist(1, 1000, "all", this, true);
                break;
            case "getCRlist":
                CRListBean.DataBean.ResultBean.CrcandidatesinfoBean curentNode = null;
                List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> curentAllList = ((CRListBean) baseEntity).getData().getResult().getCrcandidatesinfo();
                if (!AppUtlis.isNullOrEmpty(cid)) {
                    for (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean : curentAllList) {
                        if (cid.equalsIgnoreCase(bean.getCid())) {
                            curentNode = bean;
                        }
                    }
                }

                Bundle bundle = new Bundle();
                bundle.putParcelable("crStatusBean", crStatusBean);
                bundle.putParcelable("curentNode", curentNode);
                start(CRManageFragment.class, bundle);
                break;
        }
    }

    private void electedStatus() {
        if (!AppUtlis.isNullOrEmpty(cid)
                && !AppUtlis.isNullOrEmpty(status)
                && (status.equalsIgnoreCase("Terminated")
                || status.equalsIgnoreCase("Impeached")
                || status.equalsIgnoreCase("Returned"))) {
            isVoting = true;
        }
    }

    private void setRcViewData(PastCtBean pastCtBean) {
        if (null == pastCtBean) return;
        List<PastCtBean.DataBean> datas = pastCtBean.getData();
        if (datas == null || datas.size() <= 0) return;
        if (null == list) {
            list = new ArrayList<>();
        } else {
            list.clear();
        }

        for (int i = datas.size() - 1; i >= 0; i--) {
            list.add(datas.get(i));
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
    public void onManagerClick(int position, String type) {
        if (AppUtlis.isNullOrEmpty(type)) return;
        if (type.equalsIgnoreCase("VOTING")) {
            ctManagePresenter.getRegisteredCRInfo(wallet.getWalletId(), MyWallet.ELA, this);
        } else {
            Bundle bundle = new Bundle();
            bundle.putString("status", status);
            bundle.putString("cid", cid);
            bundle.putString("type", type);
            start(CtManagerFragment.class, bundle);
        }
    }

    @OnClick({R.id.iv_title_right})
    public void onClick(View view) {
        Bundle bundle = new Bundle();
        bundle.putString("id", "");
        bundle.putString("did", wallet.getDid().replace("did:elastos:", ""));
        start(SecretaryCtDetailFragment.class, bundle);
    }
}
