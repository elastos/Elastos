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

package org.elastos.wallet.ela.ui.find;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.committee.PastCtListFragment;
import org.elastos.wallet.ela.ui.committee.bean.PastCtBean;
import org.elastos.wallet.ela.ui.committee.presenter.PastCtPresenter;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.crvote.CRListFragment;
import org.elastos.wallet.ela.ui.crvote.bean.FindListBean;
import org.elastos.wallet.ela.ui.find.adapter.FindListRecAdapter;
import org.elastos.wallet.ela.ui.proposal.ProposalFragment;
import org.elastos.wallet.ela.ui.vote.SuperNodeListFragment;
import org.elastos.wallet.ela.utils.AppUtlis;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;

public class FindFragment extends BaseFragment implements CommonRvListener, NewBaseViewData {
    @BindView(R.id.iv_title_left)
    ImageView ivTitleLeft;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.rv)
    RecyclerView rv;
    private FindListRecAdapter adapter;
    private List<FindListBean> list;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_find;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.social));
        ivTitleLeft.setVisibility(View.GONE);
        setView();
    }

    public void showCrVoteItem(List<PastCtBean.DataBean> datas) {
        for (PastCtBean.DataBean dataBean : datas) {
            String status = dataBean.getStatus();
            if (!AppUtlis.isNullOrEmpty(status) && status.equalsIgnoreCase("VOTING")) {
                FindListBean bean4 = new FindListBean();
                bean4.setResouceId(R.mipmap.found_cr_vote);
                bean4.setUpText(getString(R.string.findlistup4));
                bean4.setDownText(getString(R.string.findlistdown4));
                list.add(bean4);
                setRecycleView();
                return;
            }
        }
    }

    private void setView() {
        list = new ArrayList<>();
        FindListBean bean1 = new FindListBean();
        bean1.setResouceId(R.mipmap.found_dpos_icon);
        bean1.setUpText(getString(R.string.supernode_election));
        bean1.setDownText(getString(R.string.findlistdown1));

        FindListBean bean2 = new FindListBean();
        bean2.setResouceId(R.mipmap.found_cr_proposal);
        bean2.setUpText(getString(R.string.findlistup2));
        bean2.setDownText(getString(R.string.findlistdown2));

        FindListBean bean3 = new FindListBean();
        bean3.setResouceId(R.mipmap.found_cr_committee);
        bean3.setUpText(getString(R.string.findlistup3));
        bean3.setDownText(getString(R.string.findlistdown3));

        list.add(bean1);
        list.add(bean2);
        list.add(bean3);

        //  list.add(R.mipmap.found_card_id);
        //list.add(R.mipmap.found_card_paradrop);
//        presenter = new FindPresenter();
//        presenter.getSupportedChains(wallet.getWalletId(), MyWallet.ELA, this);
        setRecycleView();
    }

    private boolean hasRefresh = false;

    public void refreshView() {
        if (!hasRefresh) {
            new PastCtPresenter().getCouncilTerm(this);
        }
    }

    public static FindFragment newInstance() {
        Bundle args = new Bundle();
        FindFragment fragment = new FindFragment();
        fragment.setArguments(args);
        return fragment;
    }

    private void setRecycleView() {
        if (adapter == null) {
            adapter = new FindListRecAdapter(getContext(), list);
            rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv.setAdapter(adapter);
            adapter.setCommonRvListener(this);

        } else {
            adapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onRvItemClick(int position, Object o) {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        if (wallet.getType() == 2 || wallet.getType() == 3) {
            showToast(getString(R.string.notsupportvoteformultwalllet));
            return;
        }
        if (position == 0) {
            ((BaseFragment) getParentFragment()).start(SuperNodeListFragment.class);
        } else if (position == 1) {
            //社区提案
            ((BaseFragment) getParentFragment()).start(ProposalFragment.class);
        } else if (position == 2) {
            ((BaseFragment) getParentFragment()).start(PastCtListFragment.class);
        } else if (position == 3) {
            ((BaseFragment) getParentFragment()).start(CRListFragment.class);
        }
    }


    /**
     * 处理回退事件
     *
     * @return
     */
    @Override
    public boolean onBackPressedSupport() {
        return closeApp();
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getCouncilTerm":
                hasRefresh = true;
                List<PastCtBean.DataBean> data = ((PastCtBean) baseEntity).getData();
                if (data == null || data.size() == 0) {
                    return;
                }
                showCrVoteItem(data);
                break;
        }
    }
}
