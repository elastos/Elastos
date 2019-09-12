package org.elastos.wallet.ela.ui.vote.SuperNodeList;


import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
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
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.find.presenter.VoteFirstPresenter;
import org.elastos.wallet.ela.ui.find.viewdata.RegisteredProducerInfoViewData;
import org.elastos.wallet.ela.ui.vote.ElectoralAffairs.ElectoralAffairsFragment;
import org.elastos.wallet.ela.ui.vote.ElectoralAffairs.VoteListPresenter;
import org.elastos.wallet.ela.ui.vote.ElectoralAffairs.VotelistViewData;
import org.elastos.wallet.ela.ui.vote.NodeCart.NodeCartFragment;
import org.elastos.wallet.ela.ui.vote.NodeInformation.NodeInformationFragment;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.ui.vote.myVote.MyVoteFragment;
import org.elastos.wallet.ela.ui.vote.signupfor.SignUpForFragment;
import org.elastos.wallet.ela.ui.vote.signupfor.SignUpPresenter;
import org.elastos.wallet.ela.utils.DividerItemDecoration;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.SPUtil;

import java.util.ArrayList;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 超级节点选举
 */
public class SuperNodeListFragment extends BaseFragment implements BaseQuickAdapter.OnItemClickListener, CommmonStringWithMethNameViewData, VotelistViewData, RegisteredProducerInfoViewData, OnRefreshListener {


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
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    public String type = "1";//1.正常投票 2. 复投
    SignUpPresenter signUpPresenter = new SignUpPresenter();
    private SuperNodeListAdapter1 adapter1;
    private String publicKey;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_super_node_list;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.supernode_election), getString(R.string.voting_rules));

        //presenter.getVotedProducerList(wallet.getWalletId(), MyWallet.ELA, this);
        srl.setOnRefreshListener(this);
        new VoteListPresenter().votelistbean("1", this);

        if (wallet.getType() != 0) {
            tv_signupfor.setVisibility(View.GONE);
        } else {
            //获取选举状态
            new VoteFirstPresenter().getRegisteredProducerInfo(wallet.getWalletId(), MyWallet.ELA, this);
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
                bundle.putString("type", type);
                bundle.putSerializable("netList", netList);
                start(NodeCartFragment.class, bundle);
                break;
            case R.id.tv_signupfor:
                if (tv_signupfor.getText().equals(getString(R.string.sign_up_for))) {
                    start(SignUpForFragment.newInstance());
                } else {
                    bundle = new Bundle();
                    bundle.putString("status", status);
                    bundle.putString("info", info);
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

    public static SuperNodeListFragment newInstance() {
        Bundle args = new Bundle();
        SuperNodeListFragment fragment = new SuperNodeListFragment();
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onItemClick(BaseQuickAdapter adapter, View view, int position) {
        Bundle bundle = new Bundle();
        bundle.putString("zb", zb);
        bundle.putString("type", type);
        bundle.putSerializable("bean", netList.get(position));
        bundle.putSerializable("netList", netList);
        start(NodeInformationFragment.class, bundle);
    }

    String zb;//占有率
    boolean is = false;//是否有自已的选举


    private void setRecyclerview(boolean is) {
        if (adapter == null) {
            recyclerview.setLayoutManager(new GridLayoutManager(getContext(), 3));
            DividerItemDecoration decoration = new DividerItemDecoration(getActivity(), DividerItemDecoration.BOTH_SET, 10, R.color.transparent);
            recyclerview.addItemDecoration(decoration);
            adapter = new SuperNodeListAdapter(this, netList, is);
            adapter.setOnItemClickListener(this);
            recyclerview.setAdapter(adapter);
        } else {
            adapter.notifyDataSetChanged();
        }
    }

    private void setRecyclerview1(boolean is) {
        if (adapter1 == null) {
            recyclerview1.setLayoutManager(new LinearLayoutManager(getContext()));
            adapter1 = new SuperNodeListAdapter1(this, netList, is);
            adapter1.setOnItemClickListener(this);
            recyclerview1.setAdapter(adapter1);
        } else {
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
                if (netList != null) {

                    for (int i = 0; i < netList.size(); i++) {
                        if (netList.get(i).getOwnerpublickey().equals(data)) {
                            VoteListBean.DataBean.ResultBean.ProducersBean temp = netList.get(i);
                            netList.remove(i);
                            netList.add(0, temp);
                            is = true;
                        }
                    }

                }
                setRecyclerview(is);
                setRecyclerview1(is);
                break;
        }
    }

    @Override
    public void onGetVoteList(VoteListBean dataResponse) {
        if (netList == null) {
            netList = new ArrayList<>();
        } else {
            netList.clear();
        }
        netList.addAll(dataResponse.getData().getResult().getProducers());
        tvNodenum.setText(netList.size() + "");
        zb = dataResponse.getData().getResult().getTotalvoterate();
        tv_zb.setText(NumberiUtil.numberFormat(Double.parseDouble(zb) * 100 + "", 2) + "%");
        tv_num.setText(dataResponse.getData().getResult().getTotalvotes().split("\\.")[0]);//totalvotes": "

        //0 普通单签 1单签只读 2普通多签 3多签只读
        if (wallet.getType() == 0 || wallet.getType() == 1) {
            //获取公钥
            if (TextUtils.isEmpty(publicKey))
                signUpPresenter.getPublicKeyForVote(wallet.getWalletId(), MyWallet.ELA, this);

        } else {
            setRecyclerview(is);
            setRecyclerview1(is);
        }


    }

    private String status;
    private String info;

    @Override
    public void onGetRegisteredProducerInfo(String data) {
        JSONObject jsonObject = JSON.parseObject(data);
        status = jsonObject.getString("Status");
        info = jsonObject.getString("Info");
        if (!TextUtils.isEmpty(status)) {
            switch (status) {
                case "Unregistered":
                    tv_signupfor.setText(getString(R.string.sign_up_for));
                    tv_signupfor.setVisibility(View.VISIBLE);
                    tv_signupfor.setCompoundDrawables(null, getDrawable(R.mipmap.vote_attend), null, null);
                    break;
                case "ReturnDeposit":
                    tv_signupfor.setVisibility(View.GONE);
                    break;
                case "Canceled":
                case "Registered":
                    tv_signupfor.setText(getString(R.string.electoral_affairs));
                    tv_signupfor.setVisibility(View.VISIBLE);
                    tv_signupfor.setCompoundDrawables(null, getDrawable(R.mipmap.vote_management), null, null);

                    break;

            }

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
        new VoteListPresenter().votelistbean("1", this);

    }
}
