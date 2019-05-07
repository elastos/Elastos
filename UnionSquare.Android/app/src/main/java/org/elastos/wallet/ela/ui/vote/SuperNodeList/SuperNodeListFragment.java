package org.elastos.wallet.ela.ui.vote.SuperNodeList;


import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;

import com.chad.library.adapter.base.BaseQuickAdapter;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.vote.ElectoralAffairs.VoteListPresenter;
import org.elastos.wallet.ela.ui.vote.ElectoralAffairs.VotelistViewData;
import org.elastos.wallet.ela.ui.vote.NodeCart.NodeCartFragment;
import org.elastos.wallet.ela.ui.vote.NodeInformation.NodeInformationFragment;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.ui.vote.myVote.MyVoteFragment;
import org.elastos.wallet.ela.ui.vote.signupfor.SignUpPresenter;
import org.elastos.wallet.ela.utils.DividerItemDecoration;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.SPUtil;
import com.qmuiteam.qmui.layout.QMUILinearLayout;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 超级节点选举
 */
public class SuperNodeListFragment extends BaseFragment implements BaseQuickAdapter.OnItemClickListener, CommmonStringWithMethNameViewData, VotelistViewData {


    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.recyclerview)
    RecyclerView recyclerview;
    ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean> netList = new ArrayList();
    private SuperNodeListAdapter adapter;
    @BindView(R.id.qmuilinearlayout)
    QMUILinearLayout qmuilinearlayout;
    @BindView(R.id.tv_zb)
    TextView tv_zb;
    @BindView(R.id.tv_num)
    TextView tv_num;
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    public String type = "1";//1.正常投票 2. 复投
    SignUpPresenter signUpPresenter = new SignUpPresenter();

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_super_node_list;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.supernode_election));
        //获取公钥
        signUpPresenter.getPublicKeyForVote(wallet.getWalletId(), MyWallet.ELA, this);
        recyclerview.setLayoutManager(new GridLayoutManager(getContext(), 3));
        DividerItemDecoration decoration = new DividerItemDecoration(getActivity(), DividerItemDecoration.BOTH_SET, 10, R.color.transparent);
        recyclerview.addItemDecoration(decoration);
        //presenter.getVotedProducerList(wallet.getWalletId(), MyWallet.ELA, this);
    }

    @OnClick({R.id.tv_myvote, R.id.tv_voting_rules, R.id.tv_going_to_vote})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_myvote:
                Bundle bundle = new Bundle();
                bundle.putString("zb", zb);
                bundle.putSerializable("netList",  netList);
                start(MyVoteFragment.class, bundle);
                break;
            case R.id.tv_voting_rules:
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
                Bundle bundle1 = new Bundle();
                bundle1.putString("zb", zb);
                bundle1.putString("type", type);
                bundle1.putSerializable("netList",  netList);
                start(NodeCartFragment.class, bundle1);
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
        bundle.putSerializable("bean", (Serializable) netList.get(position));
        bundle.putSerializable("netList", (Serializable) netList);
        start(NodeInformationFragment.class, bundle);
    }

    String zb;//占有率
    int pos=-1;//自已的投票排第几
    boolean is = false;//是否有自已的选举


    private void setRecyclerview(int pos, boolean is) {
        adapter = new SuperNodeListAdapter(getContext(), netList, pos, is);
        adapter.setOnItemClickListener(this);
        recyclerview.setAdapter(adapter);
    }

    String publicKey;

    //判断是否投过票
    @Override
    public void onGetCommonData(String methodname, String data) {
        switch (methodname) {

            //获取钱包公钥
            case "getPublicKeyForVote":
                publicKey = data;
                new VoteListPresenter().votelistbean("1", this);
                break;
        }
    }

    @Override
    public void onGetVoteList(VoteListBean dataResponse) {
        netList.addAll(dataResponse.getData().getResult().getProducers());
        //有自已的投票就排第一
        if (publicKey != null && netList != null) {
            for (int i = 0; i < netList.size(); i++) {
                if (netList.get(i).getOwnerpublickey().equals(publicKey)) {
                    //VoteListBean.DataBean.ResultBean.ProducersBean temp = list.get(i);
                    // list.remove(i);
                    //list.add(0, temp);
                    //pos = i + 1;
                    pos = i;
                    is = true;
                }
            }
        }
        setRecyclerview(pos, is);
        zb = dataResponse.getData().getResult().getTotalvoterate();
        Double zb1 = Double.parseDouble(dataResponse.getData().getResult().getTotalvoterate()) * 100;
        tv_zb.setText(NumberiUtil.numberFormat(zb1 + "", 5) + "%");
        tv_num.setText(dataResponse.getData().getResult().getTotalvotes().split("\\.")[0]);//totalvotes": "
    }
}
