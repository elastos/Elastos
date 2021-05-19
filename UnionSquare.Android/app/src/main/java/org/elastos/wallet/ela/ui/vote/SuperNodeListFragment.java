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

package org.elastos.wallet.ela.ui.vote;


import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.chad.library.adapter.base.BaseQuickAdapter;
import com.qmuiteam.qmui.layout.QMUILinearLayout;
import com.scwang.smartrefresh.layout.SmartRefreshLayout;
import com.scwang.smartrefresh.layout.api.RefreshLayout;
import com.scwang.smartrefresh.layout.listener.OnRefreshListener;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.vote.ElectoralAffairs.ElectoralAffairsFragment;
import org.elastos.wallet.ela.ui.vote.ElectoralAffairs.VoteListPresenter;
import org.elastos.wallet.ela.ui.vote.NodeCart.NodeCartFragment;
import org.elastos.wallet.ela.ui.vote.NodeInformation.NodeInformationFragment;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.SuperNodeListAdapter;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.SuperNodeListAdapter1;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.ui.vote.myVote.MyVoteFragment;
import org.elastos.wallet.ela.ui.vote.signupfor.SignUpForFragment;
import org.elastos.wallet.ela.ui.vote.signupfor.SignUpPresenter;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.DividerItemDecoration;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.SPUtil;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 超级节点选举
 */
public class SuperNodeListFragment extends BaseFragment implements BaseQuickAdapter.OnItemClickListener, CommmonStringWithMethNameViewData, NewBaseViewData, OnRefreshListener {


    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.recyclerview)
    RecyclerView recyclerview;
    ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean> netList;
    @BindView(R.id.iv_swichlist)
    ImageView ivSwichlist;
    @BindView(R.id.recyclerview1)
    RecyclerView recyclerview1;
    private SuperNodeListAdapter adapter;
    @BindView(R.id.qmuilinearlayout)
    QMUILinearLayout qmuilinearlayout;
    @BindView(R.id.tv_zb)
    TextView tv_zb;
    @BindView(R.id.tv_num)
    TextView tv_num;
    @BindView(R.id.tv_nodenum)
    TextView tvNodenum;
    @BindView(R.id.tv_signupfor)
    TextView tv_signupfor;
    @BindView(R.id.srl)
    SmartRefreshLayout srl;
    @BindView(R.id.ll_no)
    LinearLayout llNo;
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    SignUpPresenter signUpPresenter = new SignUpPresenter();
    private SuperNodeListAdapter1 adapter1;
    private String publicKey;
    private VoteListBean.DataBean.ResultBean.ProducersBean curentNode;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_super_node_list;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        CacheUtil.converDepositBean2String();
        setToobar(toolbar, toolbarTitle, getString(R.string.supernode_election), getString(R.string.voting_rules));
        netList = new ArrayList<>();
        srl.setOnRefreshListener(this);
        if (wallet.getType() != 0) {
            tv_signupfor.setVisibility(View.GONE);
        } else {
            //获取pk
            signUpPresenter.getPublicKeyForVote(wallet.getWalletId(), MyWallet.ELA, this);
        }

    }


    @OnClick({R.id.tv_myvote, R.id.tv_title_right, R.id.tv_going_to_vote, R.id.tv_signupfor, R.id.iv_swichlist})
    public void onViewClicked(View view) {
        Bundle bundle;
        switch (view.getId()) {
            case R.id.tv_myvote:
                bundle = new Bundle();
                bundle.putString("zb", zb);
                bundle.putSerializable("netList", netList);
                start(MyVoteFragment.class, bundle);
                break;
            case R.id.tv_title_right:
                int Language = new SPUtil(getContext()).getLanguage();
                if (Language == 0) {
                    Intent intent = new Intent("android.intent.action.VIEW");
                    intent.setData(Uri.parse("https://news-zh.elastos.org/亦来云dpos超级节点竞选细则/"));
                    startActivity(intent);
                } else {
                    Intent intent = new Intent("android.intent.action.VIEW");
                    intent.setData(Uri.parse("https://news.elastos.org/elastos-dpos-supernode-election-process/"));
                    startActivity(intent);
                }
                break;
            case R.id.tv_going_to_vote:
                bundle = new Bundle();
                bundle.putString("zb", zb);
                bundle.putSerializable("netList", netList);
                start(NodeCartFragment.class, bundle);
                break;
            case R.id.tv_signupfor:
                if (tv_signupfor.getText().equals(getString(R.string.sign_up_for))) {
                    start(SignUpForFragment.newInstance());
                } else {
                    bundle = new Bundle();
                    bundle.putParcelable("curentNode", curentNode);
                    start(ElectoralAffairsFragment.class, bundle);
                }
                break;
            case R.id.iv_swichlist:
                if (recyclerview.getVisibility() == View.VISIBLE) {
                    ivSwichlist.setImageResource(R.mipmap.vote_switch_squeral);
                    recyclerview.setVisibility(View.GONE);
                    recyclerview1.setVisibility(View.VISIBLE);
                } else {
                    ivSwichlist.setImageResource(R.mipmap.vote_switch_list);
                    recyclerview1.setVisibility(View.GONE);
                    recyclerview.setVisibility(View.VISIBLE);
                }
                break;
        }
    }

    @Override
    public void onItemClick(BaseQuickAdapter adapter, View view, int position) {
        Bundle bundle = new Bundle();
        bundle.putString("zb", zb);
        bundle.putSerializable("bean", netList.get(position));
        bundle.putSerializable("netList", netList);
        start(NodeInformationFragment.class, bundle);
    }

    String zb;//占有率
    boolean is = false;//是否有自已的选举


    private void setRecyclerview() {
        if (adapter == null) {
            recyclerview.setLayoutManager(new GridLayoutManager(getContext(), 3));
            DividerItemDecoration decoration = new DividerItemDecoration(getActivity(), DividerItemDecoration.BOTH_SET, 10, R.color.transparent);
            recyclerview.addItemDecoration(decoration);
            adapter = new SuperNodeListAdapter(this, netList, is);
            adapter.setOnItemClickListener(this);
            recyclerview.setAdapter(adapter);
        } else {
            adapter.setIs(is);
            adapter.notifyDataSetChanged();
        }
    }

    private void setRecyclerview1() {
        if (adapter1 == null) {
            recyclerview1.setLayoutManager(new LinearLayoutManager(getContext()));
            adapter1 = new SuperNodeListAdapter1(this, netList, is);
            adapter1.setOnItemClickListener(this);
            recyclerview1.setAdapter(adapter1);
        } else {
            adapter.setIs(is);
            adapter1.notifyDataSetChanged();
        }
    }


    //判断是否投过票
    @Override
    public void onGetCommonData(String methodname, String data) {
        switch (methodname) {

            //获取钱包公钥
            case "getPublicKeyForVote":
                publicKey = data;
                //有自已的投票就排第一
                new VoteListPresenter().getDepositVoteList("1", "all", this, true);
                break;
        }
    }

    /**
     * 重置信息  获得当前节点详情  剔除非active数据
     *
     * @param list
     */
    private void resetData(List<VoteListBean.DataBean.ResultBean.ProducersBean> list) {
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                VoteListBean.DataBean.ResultBean.ProducersBean bean = list.get(i);
                bean.setIndex(i);
                if (curentNode == null && bean.getOwnerpublickey().equals(publicKey)) {
                    curentNode = bean;
                }

                //删除非active节点
                if (!list.get(i).getState().equals("Active")) {
                    list.remove(i--);//date  remove 不影响netlist  date修改影响netlist
                }

            }

        }
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {

            case "getDepositVoteList":
                onGetDepositVoteList((VoteListBean) baseEntity);
                break;


        }
    }

    private void onGetDepositVoteList(VoteListBean dataResponse) {
        zb = dataResponse.getData().getResult().getTotalvoterate();//全局占有率
        tv_zb.setText(NumberiUtil.numberFormat(Double.parseDouble(zb) * 100 + "", 2) + "%");
        tv_num.setText(dataResponse.getData().getResult().getTotalvotes().split("\\.")[0]);//totalvotes": "
        List<VoteListBean.DataBean.ResultBean.ProducersBean> curentList = dataResponse.getData().getResult().getProducers();

        resetData(curentList);

        if (netList == null) {
            netList = new ArrayList<>();
        } else {
            netList.clear();
        }
        if (curentList != null && curentList.size() != 0) {
            netList.addAll(curentList);
            tvNodenum.setText(netList.size() + "");
            if (curentNode == null) {
                tv_signupfor.setText(getString(R.string.sign_up_for));
                tv_signupfor.setVisibility(View.VISIBLE);
                tv_signupfor.setCompoundDrawables(null, getDrawable(R.mipmap.vote_attend), null, null);

            } else {
                switch (curentNode.getState()) {
                    case "Returned":
                        tv_signupfor.setVisibility(View.GONE);
                        break;
                    case "Active":
                        is = true;
                        if (netList.indexOf(curentNode) != 0) {
                            netList.remove(curentNode);
                            netList.add(0, curentNode);
                        }
                    case "Pending":
                    case "Canceled":
                        tv_signupfor.setText(getString(R.string.electoral_affairs));
                        tv_signupfor.setVisibility(View.VISIBLE);
                        tv_signupfor.setCompoundDrawables(null, getDrawable(R.mipmap.vote_management), null, null);

                        break;


                }
            }
            if (netList == null || netList.size() == 0) {
                llNo.setVisibility(View.VISIBLE);
            } else {
                llNo.setVisibility(View.GONE);
            }
            setRecyclerview();
            setRecyclerview1();
        }
    }

    private Drawable getDrawable(int id) {
        Drawable drawable = getResources().getDrawable(id);
        drawable.setBounds(0, 0, drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight());
        return drawable;
    }

    @Override
    public void onRefresh(RefreshLayout refreshLayout) {
        onErrorRefreshLayout(srl);
        is = false;
        curentNode = null;
        new VoteListPresenter().getDepositVoteList("1", "all", this, true);
    }

}
