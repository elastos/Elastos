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


import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.chad.library.adapter.base.BaseQuickAdapter;
import com.chad.library.adapter.base.BaseViewHolder;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetBalancePresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRMyVotePresenter;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.listener.NewWarmPromptListener;
import org.json.JSONException;
import org.json.JSONObject;

import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 我的投票
 */
public class CRMyVoteFragment extends BaseFragment implements NewBaseViewData, CommonBalanceViewData {


    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.tv_name)
    TextView tvName;
    @BindView(R.id.tv_address)
    TextView tvAddress;
    @BindView(R.id.tv_num_vote)
    TextView tvNumVote;
    @BindView(R.id.iv_type)
    ImageView ivType;
    @BindView(R.id.iv_load)
    ImageView ivLoad;
    @BindView(R.id.tv_blank)
    TextView tvBlank;
    @BindView(R.id.recyclerview)
    RecyclerView recyclerview;
    @BindView(R.id.tv_goingtovote)
    TextView tvGoingtovote;
    CRMyVotePresenter presenter = new CRMyVotePresenter();
    @BindView(R.id.tv_totle)
    TextView tvTotle;
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    @BindView(R.id.ll_bgtp)
    LinearLayout ll_bgtp;
    ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> netList = new ArrayList();
    ArrayList<String> resultList = new ArrayList();

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_cr_myvote;
    }


    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.my_vote));
        presenter.getVotedCRList(wallet.getWalletId(), MyWallet.ELA, this);
        tvName.setText(wallet.getWalletName());
        new CommonGetBalancePresenter().getBalance(wallet.getWalletId(), MyWallet.ELA, 2, this);
    }

    @Override
    protected void setExtraData(Bundle data) {
        netList = (ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean>) data.getSerializable("netList");
    }

    //变更投票
    @OnClick(R.id.ll_bgtp)
    public void onViewClicked() {

        ArrayList<String> list = CacheUtil.getCRProducerListString();
        if (list.size() > 0) {
            new DialogUtil().showCommonWarmPrompt1(getBaseActivity(), getString(R.string.repalcecardlist),
                    null, null, false, new NewWarmPromptListener() {
                        @Override
                        public void affireBtnClick(View view) {
                            CacheUtil.setCRProducerListString(resultList);
                            start(CRNodeCartFragment.class, getArguments());

                        }

                        @Override
                        public void onCancel(View view) {
                            start(CRNodeCartFragment.class, getArguments());
                        }
                    });
        } else {
            CacheUtil.setCRProducerListString(resultList);
            start(CRNodeCartFragment.class, getArguments());
        }
    }

    BigDecimal ticketSum = new BigDecimal(0);
    List<Recorder> keylist = new ArrayList();

    @Override
    public void onBalance(BalanceEntity data) {
        int num = Arith.div(data.getBalance(), MyWallet.RATE_S).intValue();
        tvNumVote.setText(getString(R.string.right_to_vote_ticket) + num);
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {

            case "getVotedCRList":
                if (netList == null || netList.size() == 0) {
                    return;
                }
                String data = ((CommmonStringEntity) baseEntity).getData();
                if (data != null && !data.equals("{}") && !data.equals("null") && !data.equals("")) {
                    recyclerview.setVisibility(View.VISIBLE);
                    tvBlank.setVisibility(View.GONE);
                    ivType.setImageResource(R.mipmap.found_vote_mine_lock);
                    ll_bgtp.setVisibility(View.VISIBLE);
                    try {
                        JSONObject jsonObject = new JSONObject(data);
                        Iterator it = jsonObject.keys();

                        while (it.hasNext()) {
                            String key = (String) it.next();

                            String value = jsonObject.getString(key);
                            ticketSum = ticketSum.add(new BigDecimal(value));
                            BigDecimal number = Arith.div(value, MyWallet.RATE_S);
                            if ((number.compareTo(new BigDecimal(1)) < 0)) {
                                keylist.add(getRecord(key, "< 1"));

                            } else {
                                keylist.add(getRecord(key, number.intValue() + ""));

                            }
                        }
                        tvTotle.setText(getString(R.string.all) + NumberiUtil.numberFormat(Arith.div(ticketSum + "", MyWallet.RATE_S), 8) + " " + getString(R.string.ticket));
                        recyclerview.setLayoutManager(new LinearLayoutManager(getContext()));
                        Collections.sort(keylist);
                        recyclerview.setAdapter(new MyVoteAdapter(keylist));
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }
                }

                break;
        }
    }


    public class MyVoteAdapter extends BaseQuickAdapter<Recorder, BaseViewHolder> {


        public MyVoteAdapter(@Nullable List<Recorder> name) {
            super(R.layout.item_cr_myvote, name);

        }

        @Override
        protected void convert(BaseViewHolder helper, Recorder item) {
            helper.setText(R.id.tv_name, item.name);
            helper.setText(R.id.tv_ticketnum, item.ticketNum + mContext.getString(R.string.ticket));
            if (item.no == Integer.MAX_VALUE) {
                helper.setText(R.id.tv_no, "- -");
                helper.setTextColor(R.id.tv_no, mContext.getResources().getColor(R.color.whiter50));
                helper.setTextColor(R.id.tv_name, mContext.getResources().getColor(R.color.whiter50));
            } else {
                helper.setTextColor(R.id.tv_no, mContext.getResources().getColor(R.color.whiter));
                helper.setTextColor(R.id.tv_name, mContext.getResources().getColor(R.color.whiter));
                helper.setText(R.id.tv_no, "NO." + item.no);

            }
        }
    }

    //获取名字
    private Recorder getRecord(String cid, String ticketNum) {
        Recorder recorder = new Recorder();
        recorder.no = Integer.MAX_VALUE;
        recorder.name = getString(R.string.invalidcr);
        for (int i = 0; i < netList.size(); i++) {
            recorder.ticketNum = ticketNum;
            if (netList.get(i).getDid().equals(cid)) {
                resultList.add(cid);
                recorder.no = (netList.get(i).getIndex() + 1);
                recorder.name = netList.get(i).getNickname();
                return recorder;
            }
        }

        return recorder;
    }

    private class Recorder implements Comparable<Recorder> {
        int no;
        String name;
        String ticketNum;

        @Override
        public int compareTo(Recorder o) {
            return this.no - o.no;
        }
    }

}
