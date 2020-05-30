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
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.fragment.AddAssetFragment;
import org.elastos.wallet.ela.ui.committee.adaper.PastCtRecAdapter;
import org.elastos.wallet.ela.ui.committee.bean.PastCtBean;
import org.elastos.wallet.ela.ui.committee.fragment.CtListFragment;
import org.elastos.wallet.ela.ui.committee.fragment.CtManagerFragment;
import org.elastos.wallet.ela.ui.committee.fragment.SecretaryCtDetailFragment;
import org.elastos.wallet.ela.ui.committee.presenter.CtManagePresenter;
import org.elastos.wallet.ela.ui.committee.presenter.PastCtPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.bean.ISubWalletListEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.bean.CrStatusBean;
import org.elastos.wallet.ela.ui.crvote.fragment.CRAgreementFragment;
import org.elastos.wallet.ela.ui.crvote.fragment.CRManageFragment;
import org.elastos.wallet.ela.ui.crvote.fragment.CRSignUpForFragment;
import org.elastos.wallet.ela.ui.did.entity.AllPkEntity;
import org.elastos.wallet.ela.ui.did.presenter.AddDIDPresenter;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;

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

    private AddDIDPresenter addDIDPresenter;

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

    }

    @Override
    protected void initView(View view) {
        ivTitleRight.setImageResource(R.mipmap.found_ct_secretary_entrance);
        tvTitle.setText(mContext.getString(R.string.ctmemberlist));
        pastCtPresenter = new PastCtPresenter();
        ctManagePresenter = new CtManagePresenter();
        addDIDPresenter = new AddDIDPresenter();
        if (!AppUtlis.isNullOrEmpty(type) && type.equalsIgnoreCase("SecretaryGeneral")) {
            ivTitleRight.setVisibility(View.VISIBLE);
        } else if(!AppUtlis.isNullOrEmpty(type)
                && (type.equalsIgnoreCase("UnelectedCouncilMember"))) {
            isVoting = true;
        } else if (!AppUtlis.isNullOrEmpty(type)
                && type.equalsIgnoreCase("CouncilMember")
                && !AppUtlis.isNullOrEmpty(depositAmount)
                && !depositAmount.trim().equalsIgnoreCase("0")) {
            isCrc = true;
        } else {
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
    private String publickey;
    private String CID;
    private String DID;
    CRListBean.DataBean.ResultBean.CrcandidatesinfoBean curentNode = null;
    ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> netList;
    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getCouncilTerm":
                setRcViewData((PastCtBean) baseEntity);
                break;
            case "getRegisteredCRInfo":
                crStatusBean = JSON.parseObject(((CommmonStringEntity) baseEntity).getData(), CrStatusBean.class);
                if (crStatusBean.getStatus().equals("Unregistered")) {
                    start(CRAgreementFragment.class);
                } else {
                    addDIDPresenter.getAllSubWallets(wallet.getWalletId(), this);
                }
                break;

            case "getAllSubWallets":
                ISubWalletListEntity subWalletListEntity = (ISubWalletListEntity) baseEntity;
                for (SubWallet subWallet : subWalletListEntity.getData()) {
                    if (subWallet.getChainId().equals(MyWallet.IDChain)) {
                        addDIDPresenter.getAllPublicKeys(wallet.getWalletId(), MyWallet.IDChain, 0, 1, this);
                        return;
                    }
                }
                showOpenDIDWarm(subWalletListEntity);
                break;

            case "getAllPublicKeys":
                AllPkEntity allPkEntity = JSON.parseObject(((CommmonStringEntity) baseEntity).getData(), AllPkEntity.class);

                if (allPkEntity.getPublicKeys() == null || allPkEntity.getPublicKeys().size() == 0) {
                    return;
                }
                publickey = allPkEntity.getPublicKeys().get(0);
//                addDIDPresenter.getCIDByPublicKey(wallet.getWalletId(), publickey, this);
                ctManagePresenter.getDIDByPublicKey(wallet.getWalletId(), publickey, this);
                break;

//            case "getCIDByPublicKey":
//                CID = ((CommmonStringEntity) baseEntity).getData();
//                ctManagePresenter.getCRlist(1, 1000, "all", this, true);
//                break;

            case "getDIDByPublicKey":
                DID = ((CommmonStringEntity) baseEntity).getData();
                ctManagePresenter.getCRlist(1, 1000, "all", this, true);
                break;

            case "getCRlist":
                List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> curentAllList = ((CRListBean) baseEntity).getData().getResult().getCrcandidatesinfo();
                if (!AppUtlis.isNullOrEmpty(DID)) {
                    for (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean : curentAllList) {
                        if (DID.equalsIgnoreCase(bean.getDid())) {
                            curentNode = bean;
                        }
                    }
                }
                onGetVoteList(curentAllList);
                Bundle bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                if (crStatusBean.getStatus().equals("Unregistered")) {
                    bundle.putString("CID", CID);
                    bundle.putString("publickey", publickey);
                    bundle.putSerializable("netList", netList);
                    start(CRSignUpForFragment.class, bundle);
                } else {
                    bundle.putParcelable("crStatusBean", crStatusBean);
                    bundle.putParcelable("curentNode", curentNode);
                    start(CRManageFragment.class, bundle);
                }
                break;
        }
    }

    private int pageNum = 1;
    private final int pageSize = 1000;//基本没分页了
    boolean is = false;//是否有自已的选举
    public void onGetVoteList(List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> data) {
        if (netList == null) {
            netList = new ArrayList<>();
        }
        if (pageNum == 1) {
            netList.clear();
        } else if (data == null || data.size() == 0) {
            showToastMessage(getString(R.string.loadall));
            return;
        }
        if (data != null && data.size() != 0) {
            netList.addAll(data);
            //pos==-1表示未移除过 先移除  并获得移除的位置  待添加
            //!curentNode.getState().equals("Active")的已经移除了
            int pos = netList.indexOf(curentNode);
            if (curentNode != null && curentNode.getState().equals("Active") && pos != -1 && pos != 0) {
                //curentNode还在netList中 直接contaion耗费内存
                netList.remove(curentNode);
            }
            //只有active  并且Registered时候添加
            if (!is && curentNode != null && crStatusBean.getStatus().equals("Registered") && curentNode.getState().equals("Active")) {
                if (netList.indexOf(curentNode) != 0) {
                    netList.add(0, curentNode);
                }
                is = true;
            }
        }
        pageNum++;
    }

    private void showOpenDIDWarm(ISubWalletListEntity subWalletListEntity) {
        new DialogUtil().showCommonWarmPrompt(getBaseActivity(), getString(R.string.noidchainopenornot),
                getString(R.string.toopen), getString(R.string.cancel), false, new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        Bundle bundle = new Bundle();
                        bundle.putString("walletId", wallet.getWalletId());
                        ArrayList<String> chainIds = new ArrayList<>();
                        for (SubWallet iSubWallet : subWalletListEntity.getData()) {
                            chainIds.add(iSubWallet.getChainId());
                        }
                        bundle.putStringArrayList("chainIds", chainIds);
                        start(AddAssetFragment.class, bundle);
                    }
                });
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
