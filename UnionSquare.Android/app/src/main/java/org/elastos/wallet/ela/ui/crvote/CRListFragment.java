package org.elastos.wallet.ela.ui.crvote;


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
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.crvote.adapter.CRListAdapter;
import org.elastos.wallet.ela.ui.crvote.adapter.CRListAdapter1;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.fragment.CRInformationFragment;
import org.elastos.wallet.ela.ui.crvote.fragment.CRManageFragment;
import org.elastos.wallet.ela.ui.crvote.fragment.CRMyVoteFragment;
import org.elastos.wallet.ela.ui.crvote.fragment.CRNodeCartFragment;
import org.elastos.wallet.ela.ui.crvote.fragment.CRSignUpForFragment;
import org.elastos.wallet.ela.ui.crvote.presenter.CRlistPresenter;
import org.elastos.wallet.ela.utils.DividerItemDecoration;
import org.elastos.wallet.ela.utils.SPUtil;

import java.util.ArrayList;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 超级节点选举
 */
public class CRListFragment extends BaseFragment implements BaseQuickAdapter.OnItemClickListener, OnRefreshListener, NewBaseViewData {


    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.recyclerview)
    RecyclerView recyclerview;
    ArrayList<CRListBean.DataBean.ResultBean.ProducersBean> netList;
    @BindView(R.id.iv_swichlist)
    ImageView ivSwichlist;
    @BindView(R.id.recyclerview1)
    RecyclerView recyclerview1;

    @BindView(R.id.tv_signupfor)
    TextView tv_signupfor;
    @BindView(R.id.srl)
    SmartRefreshLayout srl;
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    CRlistPresenter presenter;
    private CRListAdapter1 adapter1;
    private CRListAdapter adapter;
    private CRListBean.DataBean.ResultBean.ProducersBean curentNode;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_cr_list;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.supernode_election), getString(R.string.voting_rules));
        //获取公钥
        presenter = new CRlistPresenter();
        presenter.getCROwnerPublicKey(wallet.getWalletId(), MyWallet.ELA, this);
        //presenter.getVotedProducerList(wallet.getWalletId(), MyWallet.ELA, this);
        onErrorRefreshLayout(srl);
        srl.setOnRefreshListener(this);
        if (wallet.getType() != 0) {
            tv_signupfor.setVisibility(View.GONE);
        } else {
            //获取选举状态
            presenter.getRegisteredCRInfo(wallet.getWalletId(), MyWallet.ELA, this);
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
                start(CRMyVoteFragment.class, bundle);
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
                start(CRNodeCartFragment.class, bundle);
                break;
            case R.id.tv_signupfor:
                if (tv_signupfor.getText().equals(getString(R.string.sign_up_for))) {
                    start(CRSignUpForFragment.class);
                } else {
                    bundle = new Bundle();
                    bundle.putString("status", status);
                    bundle.putString("info", info);
                    bundle.putSerializable("curentNode", curentNode);
                    bundle.putString("ownerPublicKey", publicKey);
                    start(CRManageFragment.class, bundle);
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

    public static CRListFragment newInstance() {
        Bundle args = new Bundle();
        CRListFragment fragment = new CRListFragment();
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onItemClick(BaseQuickAdapter adapter, View view, int position) {
        Bundle bundle = new Bundle();
        bundle.putString("zb", zb);
        bundle.putSerializable("bean", netList.get(position));
        bundle.putSerializable("netList", netList);
        start(CRInformationFragment.class, bundle);
    }

    String zb;//占有率
    int pos = -1;//自已的投票排第几
    boolean is = false;//是否有自已的选举


    private void setRecyclerview(int pos, boolean is) {
        if (adapter == null) {
            recyclerview.setLayoutManager(new GridLayoutManager(getContext(), 2));
            DividerItemDecoration decoration = new DividerItemDecoration(getActivity(), DividerItemDecoration.BOTH_SET, 10, R.color.transparent);
            recyclerview.addItemDecoration(decoration);
            adapter = new CRListAdapter(this, netList, pos, is);
            adapter.setOnItemClickListener(this);
            recyclerview.setAdapter(adapter);
        } else {
            adapter.notifyDataSetChanged();
        }
    }

    private void setRecyclerview1(int pos, boolean is) {
        if (adapter1 == null) {
            recyclerview1.setLayoutManager(new LinearLayoutManager(getContext()));
            adapter1 = new CRListAdapter1(this, netList, pos, is);
            adapter1.setOnItemClickListener(this);
            recyclerview1.setAdapter(adapter1);
        } else {
            adapter1.notifyDataSetChanged();
        }
    }

    String publicKey;


    public void onGetVoteList(CRListBean dataResponse) {
        if (netList == null) {
            netList = new ArrayList<>();
        } else {
            netList.clear();
        }
        netList.addAll(dataResponse.getData().getResult().getProducers());
        //tvNodenum.setText(netList.size() + "");
        //有自已的投票就排第一
        if (publicKey != null && netList != null) {
            for (int i = 0; i < netList.size(); i++) {
                if (netList.get(i).getOwnerpublickey().equals(publicKey)) {
                    curentNode = netList.get(i);
                    // list.remove(i);
                    //list.add(0, temp);
                    //pos = i + 1;
                    pos = i;
                    is = true;
                }
            }
        }
        setRecyclerview(pos, is);
        setRecyclerview1(pos, is);
        zb = dataResponse.getData().getResult().getTotalvoterate();
        //Double zb1 = Double.parseDouble(dataResponse.getData().getResult().getTotalvoterate()) * 100;
        //tv_zb.setText(NumberiUtil.numberFormat(zb1 + "", 2) + "%");
        //tv_num.setText(dataResponse.getData().getResult().getTotalvotes().split("\\.")[0]);//totalvotes": "
    }

    private String status;
    private String info;


    private Drawable getDrawable(int id) {
        Drawable drawable = getResources().getDrawable(id);
        drawable.setBounds(0, 0, drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight());
        return drawable;
    }

    @Override
    public void onRefresh(RefreshLayout refreshLayout) {
        if (publicKey == null) {
            presenter.getCROwnerPublicKey(wallet.getWalletId(), MyWallet.ELA, this);
        } else {
            presenter.getCRlist("1", this);
        }

    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

        switch (methodName) {
            case "getCROwnerPublicKey":

                publicKey = ((CommmonStringEntity) baseEntity).getData();
                presenter.getCRlist("1", this);
                break;
            case "getCRlist":
                onGetVoteList((CRListBean) baseEntity);
                break;
            case "getRegisteredCRInfo":

                JSONObject jsonObject = JSON.parseObject(((CommmonStringEntity) baseEntity).getData());
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
                break;
        }
    }
}
