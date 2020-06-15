/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
import org.elastos.wallet.ela.ui.committee.bean.CtDetailBean;
import org.elastos.wallet.ela.ui.committee.bean.PastCtBean;
import org.elastos.wallet.ela.ui.committee.fragment.CtListFragment;
import org.elastos.wallet.ela.ui.committee.fragment.CtManagerFragment;
import org.elastos.wallet.ela.ui.committee.fragment.SecretaryCtDetailFragment;
import org.elastos.wallet.ela.ui.committee.presenter.CtDetailPresenter;
import org.elastos.wallet.ela.ui.committee.presenter.CtManagePresenter;
import org.elastos.wallet.ela.ui.committee.presenter.PastCtPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.bean.ISubWalletListEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.crvote.bean.CrStatusBean;
import org.elastos.wallet.ela.ui.crvote.fragment.CRAgreementFragment;
import org.elastos.wallet.ela.ui.did.presenter.AddDIDPresenter;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * List of past members 委员会的首页
 */
public class PastCtListFragment extends BaseFragment implements NewBaseViewData, CommonRvListener, PastCtRecAdapter.ManagerListener, OnLoadMoreListener {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.recyclerview)
    RecyclerView recyclerview;
    PastCtRecAdapter adapter;
    List<PastCtBean.DataBean> pastCtDataList;
    CtManagePresenter ctManagePresenter;
    private boolean isCrc = false;
    private boolean isVoting = false;

    private AddDIDPresenter addDIDPresenter;

    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    private String did;
    private String type;
    private String name;
    private String status;
    private String cid;
    private String depositAmount;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_past_list;
    }


    @Override
    protected void initView(View view) {
        ivTitleRight.setImageResource(R.mipmap.found_ct_secretary_entrance);
        tvTitle.setText(mContext.getString(R.string.ctmemberlist));
        ctManagePresenter = new CtManagePresenter();
        addDIDPresenter = new AddDIDPresenter();
        new CtDetailPresenter().getCurrentCouncilInfo(this, wallet.getDid().replace("did:elastos:", ""), "crc");
    }

    private void setRecycleView() {
        if (adapter == null) {
            adapter = new PastCtRecAdapter(getContext(), pastCtDataList, isCrc, isVoting);
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
                pastCtDataList = ((PastCtBean) baseEntity).getData();
                //TODO test code
//                PastCtBean.DataBean bean2 = new PastCtBean.DataBean();
//                bean2.setStatus("VOTING");
//                bean2.setId("2");
//                bean2.setEndDate("1111111111");
//                bean2.setIndex(2);
//                bean2.setStartDate("2222222");
//                pastCtDataList.add(bean2);
//
//                PastCtBean.DataBean bean3 = new PastCtBean.DataBean();
//                bean3.setStatus("CURRENT");
//                bean3.setId("3");
//                bean3.setEndDate("1111111111");
//                bean3.setIndex(3);
//                bean3.setStartDate("2222222");
//                pastCtDataList.add(bean3);
//
//                isVoting = true;

                setRcViewData(pastCtDataList);
                break;
            case "getCurrentCouncilInfo":
                needDraw((CtDetailBean) baseEntity);
                setView((CtDetailBean) baseEntity);
                new PastCtPresenter().getCouncilTerm(this);
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
//                        addDIDPresenter.getAllPublicKeys(wallet.getWalletId(), MyWallet.IDChain, 0, 1, this);
                        Bundle bundle = new Bundle();
                        bundle.putString("name", name);
                        bundle.putString("status", status);
                        bundle.putString("cid", cid);
                        bundle.putString("depositAmount", depositAmount);
                        bundle.putString("type", type);
                        bundle.putString("did", did);
                        start(CtManagerFragment.class, bundle);
                        return;
                    }
                }
                showOpenDIDWarm(subWalletListEntity);
                break;

            case "getAllPublicKeys":
                break;
        }
    }

    private void needDraw(CtDetailBean ctDetailBean) {
        CtDetailBean.DataBean dataBean = ctDetailBean.getData();
        String type = dataBean.getType();
        String name = dataBean.getDidName();
        String did = dataBean.getDid();
        String cid = dataBean.getCid();
        String status = dataBean.getStatus();
        String depositAmount = dataBean.getDepositAmount();

        // 需要提取质押金的情况
        if(dataBean != null) {
            if(!AppUtlis.isNullOrEmpty(status)
                    && !AppUtlis.isNullOrEmpty(depositAmount)
                    && !depositAmount.equalsIgnoreCase("0")) {
                if(status.equals("Terminated")
                        || status.equals("Impeached")
                        || status.equals("Returned")) {
                    Bundle bundle = new Bundle();
                    bundle.putString("type", type);
                    bundle.putString("name", name);
                    bundle.putString("did", did);
                    bundle.putString("cid", cid);
                    bundle.putString("status", status);
                    bundle.putString("depositAmount", depositAmount);
                    start(CtManagerFragment.class, bundle);
                }
            }
        }

        //TODO test code
//        Bundle bundle = new Bundle();
//        bundle.putString("type", "other");
//        bundle.putString("name", "test");
//        bundle.putString("did", "xxxxxxxxxxxxxxxx");
//        bundle.putString("cid", "xxxxxxxxxxxxxxxx");
//        bundle.putString("status", "Impeached");
//        bundle.putString("depositAmount", "10000");
//        start(CtManagerFragment.class, bundle);
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

    private void setRcViewData(List<PastCtBean.DataBean> datas) {
        if (datas == null || datas.size() <= 0) return;
        Collections.reverse(pastCtDataList);
        setRecycleView();
    }

    @Override
    public void onRvItemClick(int position, Object o) {
        Bundle bundle = new Bundle();
        bundle.putInt("index", pastCtDataList.get(position).getIndex());
        start(CtListFragment.class, bundle);
    }

    @Override
    public void onLoadMore(RefreshLayout refreshLayout) {

    }

    @Override
    public void onManagerClick(int position, String type) {
        if (AppUtlis.isNullOrEmpty(type)) return;
        this.type = type;
        ctManagePresenter.getRegisteredCRInfo(wallet.getWalletId(), MyWallet.ELA, this);
    }

    @OnClick({R.id.iv_title_right})
    public void onClick(View view) {
        Bundle bundle = new Bundle();
        bundle.putString("id", "");
        bundle.putString("did", wallet.getDid().replace("did:elastos:", ""));
        start(SecretaryCtDetailFragment.class, bundle);
    }

    private void setView(CtDetailBean ctDetailBean) {
        CtDetailBean.DataBean dataBean = ctDetailBean.getData();
        type = dataBean.getType();
        name = dataBean.getDidName();
        did = dataBean.getDid();
        cid = dataBean.getCid();
        status = dataBean.getStatus();
        depositAmount = dataBean.getDepositAmount();
        if (!AppUtlis.isNullOrEmpty(type) && type.equalsIgnoreCase("SecretaryGeneral")) {
            ivTitleRight.setVisibility(View.VISIBLE);
        } else if (!AppUtlis.isNullOrEmpty(type)
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

    }
}
