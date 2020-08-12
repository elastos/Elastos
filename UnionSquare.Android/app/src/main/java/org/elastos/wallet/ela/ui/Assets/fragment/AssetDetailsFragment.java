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

package org.elastos.wallet.ela.ui.Assets.fragment;


import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.scwang.smartrefresh.layout.SmartRefreshLayout;
import com.scwang.smartrefresh.layout.api.RefreshLayout;
import com.scwang.smartrefresh.layout.listener.OnLoadMoreListener;
import com.scwang.smartrefresh.layout.listener.OnRefreshListener;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.activity.TransferActivity;
import org.elastos.wallet.ela.ui.Assets.adapter.TransferRecordRecAdapetr;
import org.elastos.wallet.ela.ui.Assets.adapter.VoteStatusRecAdapetr;
import org.elastos.wallet.ela.ui.Assets.bean.AddressListEntity;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.bean.TransferRecordEntity;
import org.elastos.wallet.ela.ui.Assets.bean.VoteStatus;
import org.elastos.wallet.ela.ui.Assets.fragment.transfer.SignFragment;
import org.elastos.wallet.ela.ui.Assets.presenter.AddressListPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.AssetDetailPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.AssetsPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetBalancePresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetTransactionPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonGetTransactionViewData;
import org.elastos.wallet.ela.ui.committee.bean.CtListBean;
import org.elastos.wallet.ela.ui.committee.bean.PastCtBean;
import org.elastos.wallet.ela.ui.committee.presenter.CtListPresenter;
import org.elastos.wallet.ela.ui.committee.presenter.PastCtPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringViewData;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.bean.CrStatusBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRlistPresenter;
import org.elastos.wallet.ela.ui.find.presenter.VoteFirstPresenter;
import org.elastos.wallet.ela.ui.find.viewdata.RegisteredProducerInfoViewData;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalSearchEntity;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalDetailPresenter;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalPresenter;
import org.elastos.wallet.ela.ui.vote.ElectoralAffairs.VoteListPresenter;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.json.JSONArray;
import org.json.JSONException;

import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

/**
 * 资产详情
 */
public class AssetDetailsFragment extends BaseFragment implements CommonRvListener, CommonGetTransactionViewData, OnRefreshListener, OnLoadMoreListener, CommonBalanceViewData, RegisteredProducerInfoViewData, RadioGroup.OnCheckedChangeListener, CommmonStringWithMethNameViewData, NewBaseViewData, CommmonStringViewData {


    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.srl)
    SmartRefreshLayout srl;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.rv1)
    RecyclerView rv1;
    @BindView(R.id.rv_vote)
    RecyclerView rv_Vote;
    @BindView(R.id.tv_chain)
    TextView tvChain;
    @BindView(R.id.tv_balance)
    TextView tvBalance;
    @BindView(R.id.tv_synctime)
    TextView tvSynctime;
    @BindView(R.id.tv_record_bg)
    TextView tvRecordBg;
    @BindView(R.id.tv_earn_bg)
    TextView tvEarnBg;
    @BindView(R.id.tv_vote_bg)
    TextView tvVoteBg;
    @BindView(R.id.tv_balanceuse)
    TextView tvBalanceuse;
    @BindView(R.id.rg_record)
    RadioGroup rgRecord;
    @BindView(R.id.tv_address)
    TextView tvAddress;
    @BindView(R.id.tv_towhole)
    TextView tvTowhole;
    @BindView(R.id.tv_copyaddress)
    TextView tvCopyaddress;
    @BindView(R.id.ll_earn)
    LinearLayout llEarn;
    @BindView(R.id.ll_vote)
    LinearLayout llVote;
    @BindView(R.id.rl_vote)
    RelativeLayout rlVote;
    @BindView(R.id.rl_record)
    RelativeLayout rlRecord;
    @BindView(R.id.rb_earn_recorder)
    RadioButton rbEarnRecorder;
    @BindView(R.id.rb_trans_recorder)
    RadioButton rbTransTecorder;
    @BindView(R.id.rb_vote_status)
    RadioButton rbVoteStatus;
    @BindView(R.id.view_line)
    View viewLine;
    @BindView(R.id.tv_vote_count)
    TextView tvVoteCount;
    @BindView(R.id.tv_vote_time)
    TextView tvVoteTime;
    Unbinder unbinder;
    private TransferRecordRecAdapetr adapter;
    private TransferRecordRecAdapetr adapter1;
    private VoteStatusRecAdapetr adapterVote;
    private List<TransferRecordEntity.TransactionsBean> list;
    private List<TransferRecordEntity.TransactionsBean> list1;
    private ArrayList<VoteStatus> listVoteStatus;
    private String chainId;
    private Wallet wallet;
    private int startCount = 0;
    private int startCount1 = 0;
    private final int pageCount = 20;
    private CommonGetTransactionPresenter presenter;
    private SubWallet subWallet;
    private AssetDetailPresenter assetDetailPresenter;
    private ProposalDetailPresenter proposalDetailPresenter;
    private ArrayList<ProposalSearchEntity.DataBean.ListBean> searchBeanList;
    private ArrayList<CtListBean.Council> councilList;
    private ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> crList;
    private ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean> depositList;
    private long currentStartTime;
    private String voteInfo;

    @Override
    protected int getLayoutId() {
        initClassicsFooter();
        initClassicsHeader();
        return R.layout.fragment_asset_details;
    }

    @Override
    protected void setExtraData(Bundle data) {

        wallet = data.getParcelable("wallet");
        subWallet = data.getParcelable("subWallet");
        chainId = subWallet.getChainId();
        tvTitle.setText(chainId);


    }

    @Override
    protected void initView(View view) {
        assetDetailPresenter = new AssetDetailPresenter();
        if (chainId.equals(MyWallet.ELA)) {
            tvChain.setText(getString(R.string.side_chain_top_up));
            viewLine.setVisibility(View.VISIBLE);
            tvTowhole.setVisibility(View.VISIBLE);
            new CRlistPresenter().getRegisteredCRInfo(wallet.getWalletId(), MyWallet.ELA, this);
            if ("true".equals(subWallet.getFiled2())) {
                assetDetailPresenter.getAllUTXOs(wallet.getWalletId(), chainId, 0, 1, "", this);
            }
        }
        presenter = new CommonGetTransactionPresenter();
        presenter.getAllTransaction(wallet.getWalletId(), chainId, startCount, pageCount, "", this);
        srl.setOnRefreshListener(this);
        srl.setOnLoadMoreListener(this);
        new CommonGetBalancePresenter().getBalance(wallet.getWalletId(), chainId, 2, this);
        //String synctime = new RealmUtil().querySubWalletSyncTime(wallet.getWalletId(), chainId);
        if (subWallet != null) {
            tvSynctime.setText(getString(R.string.lastsynctime) + DateUtil.time(subWallet.getSyncTime(), getContext()));
        }

        registReceiver();
        rgRecord.setOnCheckedChangeListener(this);
    }

    //OnBalanceChanged
    @OnClick({R.id.ll_transfer, R.id.tv_chain, R.id.ll_recipe, R.id.tv_towhole, R.id.tv_copyaddress, R.id.tv_vote_detail})
    public void onViewClicked(View view) {
        Bundle bundle = null;
        switch (view.getId()) {
            case R.id.tv_vote_detail:
                //投票状态详情
                //  conversUnactiveVote(currentStartTime, voteInfo, depositList, crList, searchBeanList, councilList);
                bundle = new Bundle();
                bundle.putLong("currentStartTime", currentStartTime);
                bundle.putString("voteInfo", voteInfo);
                bundle.putParcelableArrayList("depositList", depositList);
                bundle.putParcelableArrayList("crList", crList);
                bundle.putParcelableArrayList("searchBeanList", searchBeanList);
                bundle.putParcelableArrayList("councilList", councilList);
                bundle.putParcelableArrayList("listVoteStatus", listVoteStatus);

                start(VoteStatusDetailFragment.class, bundle);
                break;
            case R.id.ll_transfer:
                //转账
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                bundle.putString("ChainID", chainId);
                start(TransferFragment.class, bundle);
                break;
            case R.id.tv_chain:
                //充值
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                bundle.putString("ChainId", chainId);
                if (!chainId.equals(MyWallet.ELA)) {
                    start(MainChainWithDrawFragment.class, bundle);
                    break;
                }
                start(SideChainRechargeFragment.class, bundle);

                break;
            case R.id.ll_recipe:
                //收款
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                bundle.putString("ChainId", chainId);
                start(ReceiptFragment.class, bundle);
                break;
            case R.id.tv_towhole:
                //零钱换整
                assetDetailPresenter.createCombineUTXOTransaction(wallet.getWalletId(), chainId, "", false, this);
                break;
            case R.id.tv_copyaddress:
                ClipboardUtil.copyClipboar(getBaseActivity(), tvAddress.getText().toString());
                break;
        }
    }


    private void setRecycleView(TransferRecordEntity entity) {
        List<TransferRecordEntity.TransactionsBean> data = entity.getTransactions();
        if (startCount == 0 && (data == null || data.size() == 0)) {
            rv.setVisibility(View.GONE);
            tvRecordBg.setVisibility(View.VISIBLE);
            return;
        } else {
            rv.setVisibility(View.VISIBLE);
            tvRecordBg.setVisibility(View.GONE);
        }

        if (startCount == 0) {
            if (list == null) {
                list = new ArrayList<>();
            } else {
                list.clear();
            }
        } else if (data == null || data.size() == 0) {
            showToastMessage(getString(R.string.loadall));
            return;
        }
        list.addAll(data);
        if (adapter == null) {
            adapter = new TransferRecordRecAdapetr(getContext(), list, chainId);
            rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv.setAdapter(adapter);
            adapter.setCommonRvListener(this);

        } else {
            adapter.notifyDataSetChanged();
        }
        startCount += data.size();
    }

    private void setRecycleView1(TransferRecordEntity entity) {
        List<TransferRecordEntity.TransactionsBean> data = entity.getTransactions();
        if (startCount1 == 0 && (data == null || data.size() == 0)) {
            rv1.setVisibility(View.GONE);
            tvEarnBg.setVisibility(View.VISIBLE);
            return;
        } else {
            rv1.setVisibility(View.VISIBLE);
            tvEarnBg.setVisibility(View.GONE);
        }

        if (startCount1 == 0) {
            if (list1 == null) {
                list1 = new ArrayList<>();
            } else {
                list1.clear();
            }
        } else if (data == null || data.size() == 0) {
            showToastMessage(getString(R.string.loadall));
            return;
        }
        list1.addAll(data);
        if (adapter1 == null) {
            adapter1 = new TransferRecordRecAdapetr(getContext(), list1, chainId);
            rv1.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv1.setAdapter(adapter1);
            adapter1.setCommonRvListener(this);

        } else {
            adapter1.notifyDataSetChanged();
        }
        startCount1 += data.size();
    }

    private void setVoteRecycleView(String voteInfo) {

        if (TextUtils.isEmpty(voteInfo) || voteInfo.equals("null") || voteInfo.equals("[]")) {
            tvVoteBg.setVisibility(View.VISIBLE);
            llVote.setVisibility(View.GONE);
            return;
        }
        tvVoteBg.setVisibility(View.GONE);
        llVote.setVisibility(View.VISIBLE);
        if (listVoteStatus == null) {
            listVoteStatus = new ArrayList<>();
        }
        initListVoteStatus(listVoteStatus);


        conversUnactiveVote(currentStartTime, voteInfo, depositList, crList, searchBeanList, councilList);
        if (adapterVote == null) {
            adapterVote = new VoteStatusRecAdapetr(getContext(), listVoteStatus);
            rv_Vote.setLayoutManager(new GridLayoutManager(getContext(), 2));
            rv_Vote.setAdapter(adapterVote);

        } else {
            adapterVote.notifyDataSetChanged();
        }


    }

    private void initListVoteStatus(List<VoteStatus> listVoteStatus) {
        VoteStatus voteStatusDepos = new VoteStatus();
        voteStatusDepos.setIndex(0);
        voteStatusDepos.setIconID(R.mipmap.asset_vote_node);
        voteStatusDepos.setName(getString(R.string.dposnotevote));
        listVoteStatus.add(voteStatusDepos);
        VoteStatus voteStatusCR = new VoteStatus();
        voteStatusCR.setIndex(1);
        voteStatusCR.setIconID(R.mipmap.asset_vote_cr);
        voteStatusCR.setName(getString(R.string.crcvote));
        listVoteStatus.add(voteStatusCR);
        VoteStatus voteStatusImpeach = new VoteStatus();
        voteStatusImpeach.setIndex(2);
        voteStatusImpeach.setIconID(R.mipmap.asset_vote_judgement);
        voteStatusImpeach.setName(getString(R.string.crimpeach));
        listVoteStatus.add(voteStatusImpeach);
        VoteStatus voteStatusAgainst = new VoteStatus();
        voteStatusAgainst.setIndex(3);
        voteStatusAgainst.setIconID(R.mipmap.cr_vote_nay);
        voteStatusAgainst.setName(getString(R.string.communityproposalagaginstvote));
        listVoteStatus.add(voteStatusAgainst);
    }

    public void conversUnactiveVote(long currentStartTime, String voteInfo, List<VoteListBean.DataBean.ResultBean.ProducersBean> depositList,
                                    List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> crcList, List<ProposalSearchEntity.DataBean.ListBean> voteList, List<CtListBean.Council> councilList) {


        try {
            JSONArray lastVoteInfo = new JSONArray(voteInfo);
            long lastTime = 0;
            BigDecimal maxCount = new BigDecimal(0);
            for (int i = 0; i < lastVoteInfo.length(); i++) {
                org.json.JSONObject jsonObject = lastVoteInfo.getJSONObject(i);
                String type = jsonObject.getString("Type");

                long timestamp = jsonObject.getLong("Timestamp");
                if (lastTime < timestamp) {
                    lastTime = timestamp;
                }
                org.json.JSONObject votes = jsonObject.getJSONObject("Votes");
                Iterator it = votes.keys();
                JSONArray candidates = new JSONArray();
                VoteStatus voteStatus = null;
                BigDecimal count = new BigDecimal(0);
                switch (type) {
                    case "Delegate":
                        voteStatus = listVoteStatus.get(0);
                        while (it.hasNext()) {
                            String key = (String) it.next();
                            String value = votes.getString(key);
                            count = new BigDecimal(value);
                            if (depositList == null || depositList.size() == 0) {
                                candidates.put(key);
                                continue;
                            }
                            for (VoteListBean.DataBean.ResultBean.ProducersBean bean : depositList) {
                                if (bean.getOwnerpublickey().equals(key) && !bean.getState().equals("Active")) {
                                    candidates.put(key);
                                    break;
                                }
                            }

                        }
                        break;
                    case "CRC":
                        voteStatus = listVoteStatus.get(1);
                        while (it.hasNext()) {
                            String key = (String) it.next();
                            String value = votes.getString(key);
                            count = count.add(new BigDecimal(value));
                            if (timestamp < currentStartTime || crcList == null || crcList.size() == 0) {
                                candidates.put(key);
                                continue;
                            }
                            for (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean : crcList) {
                                if (bean.getDid().equals(key) && !bean.getState().equals("Active")) {
                                    candidates.put(key);
                                    break;
                                }
                            }
                        }

                        break;
                    case "CRCImpeachment"://弹劾
                        voteStatus = listVoteStatus.get(2);
                        while (it.hasNext()) {
                            String key = (String) it.next();
                            String value = votes.getString(key);
                            count = count.add(new BigDecimal(value));
                            if (timestamp < currentStartTime || councilList == null || councilList.size() == 0) {
                                candidates.put(key);
                                continue;
                            }
                            for (CtListBean.Council bean : councilList) {
                                if ((bean.getDid().equals(key) && !bean.getStatus().equals("Elected"))) {
                                    candidates.put(key);
                                    break;
                                }
                            }
                        }
                        break;
                    case "CRCProposal":
                        voteStatus = listVoteStatus.get(3);
                        while (it.hasNext()) {
                            String key = (String) it.next();
                            String value = votes.getString(key);
                            count = count.add(new BigDecimal(value));
                            if (voteList == null || voteList.size() == 0) {
                                candidates.put(key);
                                continue;
                            }
                            for (ProposalSearchEntity.DataBean.ListBean bean : voteList) {
                                if (bean.getProposalHash().equals(key) && !bean.getStatus().equals("NOTIFICATION")) {
                                    candidates.put(key);
                                    break;
                                }
                            }
                        }
                        break;


                }
                if (voteStatus != null) {
                    //默认0没有投票   1 有投票部分失效 2 有投票完全失效 3有投票无失效
                    if (candidates.length() == 0) {
                        voteStatus.setStatus(3);
                    } else if (candidates.length() == votes.length()) {
                        voteStatus.setStatus(2);
                    } else {
                        voteStatus.setStatus(1);
                    }
                    voteStatus.setCount(count);
                    if (maxCount.compareTo(count) < 0) {
                        maxCount = count;
                    }
                }


            }
            tvVoteTime.setText(getString(R.string.lastvotetime) + DateUtil.time(lastTime, getContext()));
            tvVoteCount.setText(getString(R.string.allcount) + Arith.div(maxCount, MyWallet.RATE_S, 8).longValue() + " ELA");
        } catch (JSONException e) {
            e.printStackTrace();
        }


    }

    @Override
    public void onRvItemClick(int position, Object o) {
        Bundle bundle = new Bundle();
        bundle.putString("ChainId", chainId);
        bundle.putParcelable("wallet", wallet);
        bundle.putString("TxHash", ((TransferRecordEntity.TransactionsBean) o).getTxHash());
        if (rbEarnRecorder.isChecked()) {
            bundle.putInt("recordType", 1);//收益记录
        } else {
            bundle.putInt("recordType", 0);//转账记录
        }

        start(TransferDetailFragment.class, bundle);
    }


    @Override
    public void onGetAllTransaction(String data) {
        if (srl != null) {
            srl.finishRefresh();
            srl.finishLoadMore();
        }
        TransferRecordEntity transferRecordEntity = JSON.parseObject(data, TransferRecordEntity.class);
        setRecycleView(transferRecordEntity);
    }

    @Override
    public void onRefresh(RefreshLayout refreshLayout) {
        onErrorRefreshLayout(srl);
        if (rbEarnRecorder.isChecked()) {
            startCount1 = 0;
            assetDetailPresenter.getAllCoinBaseTransaction(wallet.getWalletId(), chainId, startCount1, pageCount, "", this);
        } else if (rbTransTecorder.isChecked()) {
            startCount = 0;
            presenter.getAllTransaction(wallet.getWalletId(), chainId, startCount, pageCount, "", this);
        } else if (rbVoteStatus.isChecked()) {
            voteTag = 0;
            new ProposalPresenter().proposalSearch(-1, -1, "NOTIFICATION", null, this);
            new VoteListPresenter().getDepositVoteList("1", "all", this, true);
            new CRlistPresenter().getCRlist(-1, -1, "all", this, true);
            new CtListPresenter().getCouncilList(this, String.valueOf(1));
            new PastCtPresenter().getCouncilTerm(this);
        }
    }

    @Override
    public void onLoadMore(RefreshLayout refreshLayout) {
        onErrorRefreshLayout(srl);
        if (rbEarnRecorder.isChecked()) {
            assetDetailPresenter.getAllCoinBaseTransaction(wallet.getWalletId(), chainId, startCount1, pageCount, "", this);
        } else {
            presenter.getAllTransaction(wallet.getWalletId(), chainId, startCount, startCount + pageCount, "", this);
        }
        new AssetsPresenter().syncStart(wallet.getWalletId(), chainId, this);
    }

    @Override
    public void onBalance(BalanceEntity data) {
        if (data != null && !TextUtils.isEmpty(data.getBalance())) {
            tvBalance.setText(NumberiUtil.maxNumberFormat(Arith.div(data.getBalance(), MyWallet.RATE_S), 12) + " ELA");
        }
    }


    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.BALANCECHANGE.ordinal()) {
            SubWallet subWallet = (SubWallet) result.getObj();
            if (subWallet != null && subWallet.getBelongId().equals(wallet.getWalletId()) &&
                    subWallet.getChainId().equals(chainId)) {
                tvBalance.setText(NumberiUtil.maxNumberFormat(Arith.div(subWallet.getBalance(), MyWallet.RATE_S), 12) + " ELA");
            }
        }
        if (integer == RxEnum.UPDATAPROGRESS.ordinal()) {
            SubWallet subWallet = (SubWallet) result.getObj();
            if (subWallet != null && subWallet.getBelongId().equals(wallet.getWalletId()) &&
                    subWallet.getChainId().equals(chainId)) {
                tvSynctime.setText(getString(R.string.lastsynctime) + DateUtil.time(subWallet.getSyncTime(), getContext()));
            }
        }
        if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                }
            });

        }
        if (integer == RxEnum.TOSIGN.ordinal()) {
            //生成待签名交易
            String attributes = (String) result.getObj();
            Bundle bundle = new Bundle();
            bundle.putString("attributes", attributes);
            bundle.putParcelable("wallet", wallet);
            bundle.putInt("transType", 2);
            start(SignFragment.class, bundle);

        }
        if (integer == RxEnum.SIGNSUCCESS.ordinal()) {
            //签名成功
            String attributes = (String) result.getObj();
            Bundle bundle = new Bundle();
            bundle.putString("attributes", attributes);
            bundle.putParcelable("wallet", wallet);
            bundle.putInt("transType", 2);
            bundle.putBoolean("signStatus", true);
            start(SignFragment.class, bundle);

        }

    }

    @Override
    public void onGetRegisteredProducerInfo(String data) {
        JSONObject jsonObject = JSON.parseObject(data);
        String status = jsonObject.getString("Status");
        if (!"Unregistered".equals(status)) {
            rbEarnRecorder.setVisibility(View.VISIBLE);
            assetDetailPresenter.getOwnerAddress(wallet.getWalletId(), chainId, this);
        }
    }


    @Override
    public void onCheckedChanged(RadioGroup group, int checkedId) {
        switch (checkedId) {
            case R.id.rb_trans_recorder:

                rlRecord.setVisibility(View.VISIBLE);
                llEarn.setVisibility(View.GONE);
                rlVote.setVisibility(View.GONE);
                srl.setEnableLoadMore(true);
                break;
            case R.id.rb_earn_recorder:
                if (list1 == null) {
                    assetDetailPresenter.getAllCoinBaseTransaction(wallet.getWalletId(), chainId, startCount1, pageCount, "", this);
                }
                rlRecord.setVisibility(View.GONE);
                llEarn.setVisibility(View.VISIBLE);
                rlVote.setVisibility(View.GONE);
                srl.setEnableLoadMore(true);
                break;
            case R.id.rb_vote_status:
                if (voteTag == 0) {
                    new ProposalPresenter().proposalSearch(-1, -1, "NOTIFICATION", null, this);
                    new VoteListPresenter().getDepositVoteList("1", "all", this, true);
                    new CRlistPresenter().getCRlist(-1, -1, "all", this, true);
                    new CtListPresenter().getCouncilList(this, String.valueOf(1));
                    new PastCtPresenter().getCouncilTerm(this);

                }
                rlRecord.setVisibility(View.GONE);
                llEarn.setVisibility(View.GONE);
                rlVote.setVisibility(View.VISIBLE);
                srl.setEnableLoadMore(false);
                break;
        }
    }


    @Override
    public void onGetCommonData(String methodname, String data) {
        switch (methodname) {
            case "getOwnerAddress":
                tvAddress.setText(data);
                break;
            case "getAllUTXOs":
                JSONObject jsonObject = JSON.parseObject(data);
                if (jsonObject.containsKey("MaxCount")) {
                    int maxCount = jsonObject.getInteger("MaxCount");
                    if (maxCount > 500) {
                        exchange(maxCount);
                    }
                }
                break;
            case "createCombineUTXOTransaction":
                //零钱换整
                Intent intent = new Intent(getActivity(), TransferActivity.class);

                intent.putExtra("wallet", wallet);
                intent.putExtra("attributes", data);
                intent.putExtra("chainId", chainId);
                intent.putExtra("type", Constant.TOWHOL);
                intent.putExtra("transType", 2);
                startActivity(intent);
                break;
            case "getAllCoinBaseTransaction":
                if (srl != null) {
                    srl.finishRefresh();
                    srl.finishLoadMore();
                }
                TransferRecordEntity transferRecordEntity = JSON.parseObject(data, TransferRecordEntity.class);
                setRecycleView1(transferRecordEntity);
                break;
        }
    }

    private void exchange(int maxCount) {
        String tip = String.format(getString(R.string.whetherexchange), maxCount + "");

        new DialogUtil().showWarmPrompt1(getBaseActivity(), tip, new WarmPromptListener() {
            @Override
            public void affireBtnClick(View view) {
                assetDetailPresenter.createCombineUTXOTransaction(wallet.getWalletId(), chainId, "", false, AssetDetailsFragment.this);

            }
        });
    }

    int voteTag;//tag==4调用getvote 避免无用的调用

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getCouncilTerm":

                List<PastCtBean.DataBean> pastCtBeanList = ((PastCtBean) baseEntity).getData();
                if (pastCtBeanList != null && pastCtBeanList.size() > 0) {
                    for (PastCtBean.DataBean dataBean : pastCtBeanList) {
                        if ("CURRENT".equals(dataBean.getStatus())) {
                            currentStartTime = dataBean.getStartDate();
                            break;

                        }
                    }
                }
                getvoteInfo(voteTag++);

                break;
            case "getVoteInfo":
                //剔除非公示期的
                voteInfo = ((CommmonStringEntity) baseEntity).getData();
                setVoteRecycleView(voteInfo);


                //   JSONArray otherUnActiveVote = proposalDetailPresenter.conversUnactiveVote(currentStartTime, null, voteInfo, depositList, crList, searchBeanList, councilList);

                break;
            case "proposalSearch":
                searchBeanList = (ArrayList<ProposalSearchEntity.DataBean.ListBean>) ((ProposalSearchEntity) baseEntity).getData().getList();

                getvoteInfo(voteTag++);

                break;
            case "getCouncilList":
                councilList = (ArrayList<CtListBean.Council>) ((CtListBean) baseEntity).getData().getCouncil();
                getvoteInfo(voteTag++);
                break;
            case "getCRlist":
                crList = (ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean>) ((CRListBean) baseEntity).getData().getResult().getCrcandidatesinfo();
                getvoteInfo(voteTag++);
                break;
            case "getDepositVoteList":

                depositList = (ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean>) ((VoteListBean) baseEntity).getData().getResult().getProducers();
                getvoteInfo(voteTag++);
                break;
            case "getRegisteredCRInfo":
                CrStatusBean crStatusBean = JSON.parseObject(((CommmonStringEntity) baseEntity).getData(), CrStatusBean.class);
                String status = crStatusBean.getStatus();
                if (!"Unregistered".equals(status)) {
                    rbEarnRecorder.setVisibility(View.VISIBLE);
                    new AddressListPresenter().getAllAddress(wallet.getWalletId(), MyWallet.ELA, 0, 1, this);
                } else {
                    new VoteFirstPresenter().getRegisteredProducerInfo(wallet.getWalletId(), chainId, this);
                }
                break;
        }
    }

    private void getvoteInfo(int voteTag) {
        if (voteTag == 4) {
            if (proposalDetailPresenter == null) {
                proposalDetailPresenter = new ProposalDetailPresenter();
            }
            proposalDetailPresenter.getVoteInfo(wallet.getWalletId(), "", this);
        }
    }

    @Override
    public void onGetCommonData(String data) {
        //获得钱包地址列表
        AddressListEntity addressListEntity = JSON.parseObject(data, AddressListEntity.class);
        List<String> addressList = addressListEntity.getAddresses();
        tvAddress.setText(addressList.get(0));
        new VoteFirstPresenter().getRegisteredProducerInfo(wallet.getWalletId(), chainId, this);
    }

}
