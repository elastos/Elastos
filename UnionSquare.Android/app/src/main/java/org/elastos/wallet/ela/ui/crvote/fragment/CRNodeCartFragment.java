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

package org.elastos.wallet.ela.ui.crvote.fragment;


import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.AppCompatTextView;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.blankj.utilcode.util.ToastUtils;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.activity.TransferActivity;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.fragment.transfer.SignFragment;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetBalancePresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.committee.bean.CtListBean;
import org.elastos.wallet.ela.ui.committee.presenter.CtListPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.crvote.adapter.CRNodeCartAdapter;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRNodeCartPresenter;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalSearchEntity;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalDetailPresenter;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalPresenter;
import org.elastos.wallet.ela.ui.vote.ElectoralAffairs.VoteListPresenter;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.NewWarmPromptListener;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 节点购车车
 */
public class CRNodeCartFragment extends BaseFragment implements CommonBalanceViewData, CRNodeCartAdapter.OnViewClickListener, CRNodeCartAdapter.OnTextChangedListener, NewBaseViewData {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.tv_balance)
    AppCompatTextView tvBalance;
    @BindView(R.id.tv_avaliable)
    TextView tvAvaliable;
    @BindView(R.id.ll_bottom2)
    LinearLayout llBottom2;
    @BindView(R.id.ll_bottom1)
    LinearLayout llBottom1;
    @BindView(R.id.recyclerview)
    RecyclerView recyclerView;


    private CRNodeCartAdapter mAdapter;
    @BindView(R.id.cb_equal)
    CheckBox cbEqual;
    private ProposalDetailPresenter proposalDetailPresenter;
    @BindView(R.id.tv_amount)
    AppCompatTextView tvAmount;
    List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list = new ArrayList<>();
    @BindView(R.id.ll_blank)
    LinearLayout ll_blank;
    @BindView(R.id.cb_selectall)
    CheckBox cbSelectall;

    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();

    CRNodeCartPresenter presenter;
    ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> netList;
    private BigDecimal balance;
    private org.json.JSONArray otherUnActiveVote;
    private int transType = 1001;
    private List<ProposalSearchEntity.DataBean.ListBean> searchBeanList;
    private List<VoteListBean.DataBean.ResultBean.ProducersBean> depositList;
    private List<CtListBean.Council> councilList;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_cr_nodecart;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        netList = (ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean>) data.getSerializable("netList");

    }

    @Override
    protected void initView(View view) {
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.found_vote_edit);
        tvTitle.setText(getString(R.string.crcvote));
        if (netList == null) {
            //没有来自接口的节点列表数据
            netList = new ArrayList<>();
        }
        registReceiver();
        // 为Adapter准备数据
        initDate();
        setRecyclerView(list);
        new CommonGetBalancePresenter().getBalance(wallet.getWalletId(), MyWallet.ELA, 2, this);

        tvAvaliable.setText(getString(R.string.available) + "：" + "0 ELA");
        new ProposalPresenter().proposalSearch(-1, -1, "ALL", null, this);
        new VoteListPresenter().getDepositVoteList("1", "all", this, true);
        new CtListPresenter().getCouncilList(this, String.valueOf(1));
    }


    // 初始化数据
    private void initDate() {
        ArrayList<String> list1 = CacheUtil.getCRProducerListString();
        if (list1.size() == 0) {
            return;
        }
        ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> newlist = new ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean>();
        for (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean : netList) {
            if (list1.contains(bean.getDid())) {
                bean.setCurentBalance(null);
                newlist.add(bean);
            }
        }
        Collections.sort(newlist);
        list.clear();
        list.addAll(newlist);
        CacheUtil.setCRProducerList(list);
    }


    public void setRecyclerView(List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list) {

        if (list == null || list.size() == 0) {
            ll_blank.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
        } else {
            ll_blank.setVisibility(View.GONE);
            recyclerView.setVisibility(View.VISIBLE);
        }
        if (mAdapter == null) {
            recyclerView.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            mAdapter = new CRNodeCartAdapter(list, this);
            mAdapter.initDateStaus(false);
            recyclerView.setAdapter(mAdapter);
            mAdapter.setOnViewClickListener(this);
            mAdapter.setOnTextChangedListener(this);
        } else {
            mAdapter.notifyDataSetChanged();
        }
    }


    @OnClick({R.id.iv_title_right, R.id.tv_delete, R.id.tv_vote, R.id.cb_selectall, R.id.cb_equal})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.cb_equal:
                doCheckMuil(cbEqual.isChecked(), true);
                break;
            case R.id.cb_selectall:
                mAdapter.initDateStaus(cbSelectall.isChecked());
                mAdapter.notifyDataSetChanged();
                break;

            case R.id.iv_title_right:
                doCheckMuil(false, false);
                if (tvAvaliable.getVisibility() == View.VISIBLE) {
                    tvAvaliable.setVisibility(View.GONE);
                    tvBalance.setVisibility(View.GONE);
                    llBottom2.setVisibility(View.VISIBLE);
                    llBottom1.setVisibility(View.GONE);
                    ivTitleRight.setImageResource(R.mipmap.found_vote_finish);

                } else {
                    tvAvaliable.setVisibility(View.VISIBLE);
                    tvBalance.setVisibility(View.VISIBLE);
                    llBottom2.setVisibility(View.GONE);
                    llBottom1.setVisibility(View.VISIBLE);
                    ivTitleRight.setImageResource(R.mipmap.found_vote_edit);

                }
                break;

            case R.id.tv_delete:
                if (list == null || list.size() == 0) {
                    return;
                }
                List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> deleteList = new ArrayList();
                for (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean : list) {
                    if (bean.isChecked()) {
                        deleteList.add(bean);
                    }
                }
                list.removeAll(deleteList);
                cbSelectall.setChecked(false);
                mAdapter.notifyDataSetChanged();
                CacheUtil.setCRProducerList(list);
                break;

            case R.id.tv_vote:
                if (proposalDetailPresenter == null) {
                    proposalDetailPresenter = new ProposalDetailPresenter();
                }
                proposalDetailPresenter.getVoteInfo(wallet.getWalletId(), "", this);
                break;

        }
    }


    private void doVote() {

        Map<String, String> checkedData = mAdapter.getCheckedData();
        if (mAdapter.getCheckAndHasBalanceNum() > 36) {
            showToast(getString(R.string.max36dot));
            return;
        }
        if (checkedData == null || checkedData.size() == 0) {
            ToastUtils.showShort(getString(R.string.please_select));
            return;
        }
        String result = checkedData.toString().replace("=", ":");
        if (presenter == null) {
            presenter = new CRNodeCartPresenter();
        }
        presenter.createVoteCRTransaction(wallet.getWalletId(), MyWallet.ELA, "", result, "", otherUnActiveVote.toString(), this);
    }

    //查询余额结果
    @Override
    public void onBalance(BalanceEntity data) {
        balance = Arith.div(Arith.sub(data.getBalance(), 1000000), MyWallet.RATE_S, 8);
        if ((balance.compareTo(new BigDecimal(1)) < 0)) {
            //小于1
            if ((balance.compareTo(new BigDecimal(0)) <= 0)) {
                balance = new BigDecimal(0);
                tvBalance.setText(getString(R.string.maxvote1) + "0 ELA");
                tvAvaliable.setText(getString(R.string.available) + "：" + "0 ELA");
            } else {
                tvBalance.setText(getString(R.string.maxvote1) + "< 1 ELA");
                tvAvaliable.setText(getString(R.string.available) + "：" + "< 1 ELA");
            }
        } else {
            tvBalance.setText(getString(R.string.maxvote) + balance.intValue() + " ELA");
            tvAvaliable.setText(getString(R.string.available) + "：" + balance.intValue() + " ELA");
        }
        mAdapter.setBalance(balance);
    }


    /**
     * 一次性全部取消 或者全选时候的ui改变
     *
     * @param tag 选中还是取消选中
     * @param tag 是否检查选中的数量36
     */
    private void doCheckMuil(boolean tag, boolean checkSelectNuber) {
        if (!tag) {
            if (checkSelectNuber) {
                //检查选中的数量36的情况下取消全选
                mAdapter.initAllCurentBalance();
            }
            mAdapter.initDateStaus(false);
            mAdapter.notifyDataSetChanged();
            tvAmount.setText(getString(R.string.totle) + "0 ELA");
            tvAvaliable.setText(getString(R.string.available) + "：" + balance.intValue() + " ELA");
            cbSelectall.setChecked(false);
            cbEqual.setChecked(false);
        } else if (checkSelectNuber && mAdapter.getList().size() > 36) {
            //大于36的平均分配
            new DialogUtil().showWarmPrompt2(getBaseActivity(), "", new NewWarmPromptListener() {
                @Override
                public void affireBtnClick(View tag) {
                    // 遍历list的长度，将MyAdapter中的map值全部设为false
                    mAdapter.initDateStaus(false);
                    mAdapter.equalDataMapELA(36);
                    mAdapter.setDateStaus(36, true);
                    mAdapter.notifyDataSetChanged();
                    resetCountAndAvaliable();
                    cbSelectall.setChecked(true);
                    cbEqual.setChecked(true);
                }

                @Override
                public void onCancel(View view) {
                    cbEqual.setChecked(false);
                }
            });

        } else {
            mAdapter.initDateStaus(true);
            mAdapter.equalDataMapELA(mAdapter.getList().size());
            mAdapter.notifyDataSetChanged();
            resetCountAndAvaliable();
            cbSelectall.setChecked(true);
            cbEqual.setChecked(true);

        }


    }


    @Override
    public void onItemViewClick(CRNodeCartAdapter adapter, View clickView, int position) {
        setSelectAllStatus();
        resetCountAndAvaliable();
    }

    private void resetCountAndAvaliable() {
        tvAmount.setText(getString(R.string.totle) + NumberiUtil.numberFormat(mAdapter.getCountEla(), 8) + " ELA");
        BigDecimal countEla = mAdapter.getCountEla();
        countEla = balance.subtract(countEla);
        if (countEla.compareTo(new BigDecimal(0)) <= 0) {
            tvAvaliable.setText(getString(R.string.available) + "：" + "0 ELA");
        } else {
            if (countEla.compareTo(new BigDecimal(1)) < 0) {
                tvAvaliable.setText(getString(R.string.available) + "：" + "< 1 ELA");
            } else {
                tvAvaliable.setText(getString(R.string.available) + "：" + countEla.intValue() + " ELA");
            }

        }
    }

    /**
     * 设置下部全选按钮状态
     */
    private void setSelectAllStatus() {
        int checkSum = mAdapter.getCheckNum();
        if (list != null && list.size() > 0 && checkSum == list.size()) {
            cbSelectall.setChecked(true);

        } else {
            cbSelectall.setChecked(false);

        }
    }

    @Override
    public void onTextChanged(CRNodeCartAdapter adapter, View clickView, int position) {
        resetCountAndAvaliable();

    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getCouncilList":
                councilList = ((CtListBean) baseEntity).getData().getCouncil();
                break;
            case "proposalSearch":
                searchBeanList = ((ProposalSearchEntity) baseEntity).getData().getList();
                break;
            case "getDepositVoteList":
                depositList = ((VoteListBean) baseEntity).getData().getResult().getProducers();
                break;
            case "createVoteCRTransaction":
                //Status 0 或者没有  表述正常  其他皆为不同情况的异常
                String attributes = ((CommmonStringEntity) baseEntity).getData();
                JSONObject attributesJson = JSON.parseObject(attributes);
                String status = attributesJson.getString("DropVotes");
                if (!TextUtils.isEmpty(status) && !status.equals("[]")) {
                    transType = 1003;
                    showOpenDraftWarm(attributes);
                    break;
                }
                transType = 1001;
                goTransferActivity(attributes);
                break;
            case "getVoteInfo":
                //剔除非公示期的
                String voteInfo = ((CommmonStringEntity) baseEntity).getData();
                otherUnActiveVote = proposalDetailPresenter.conversUnactiveVote("CRC", voteInfo, depositList, netList, searchBeanList, councilList);
                doVote();
                break;

        }
    }

    private void goTransferActivity(String attributesJson) {
        Intent intent = new Intent(getActivity(), TransferActivity.class);
        intent.putExtra("amount", mAdapter.getCountEla().toPlainString());
        intent.putExtra("wallet", wallet);
        intent.putExtra("attributes", attributesJson);
        intent.putExtra("chainId", MyWallet.ELA);
        intent.putExtra("type", Constant.CRVOTE);
        intent.putExtra("transType", transType);
        startActivity(intent);
    }

    private void showOpenDraftWarm(String attributesJson) {
        new DialogUtil().showCommonWarmPrompt(getBaseActivity(), getString(R.string.notsufficientfundskeepornot),
                getString(R.string.sure), getString(R.string.cancel), false, new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        goTransferActivity(attributesJson);
                    }
                });
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                    popBackFragment();
                }
            });

        }
        if (integer == RxEnum.TOSIGN.ordinal()) {
            //生成待签名交易
            String attributes = (String) result.getObj();
            Bundle bundle = new Bundle();
            bundle.putString("attributes", attributes);
            bundle.putParcelable("wallet", wallet);
            bundle.putInt("transType", transType);
            start(SignFragment.class, bundle);

        }
        if (integer == RxEnum.SIGNSUCCESS.ordinal()) {
            //签名成功
            String attributes = (String) result.getObj();
            Bundle bundle = new Bundle();
            bundle.putString("attributes", attributes);
            bundle.putParcelable("wallet", wallet);
            bundle.putBoolean("signStatus", true);
            bundle.putInt("transType", transType);
            start(SignFragment.class, bundle);

        }
    }


}
