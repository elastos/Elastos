package org.elastos.wallet.ela.ui.vote.myVote;


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
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetBalancePresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.vote.NodeCart.NodeCartFragment;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.klog.KLog;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 我的投票
 */
public class MyVoteFragment extends BaseFragment implements CommmonStringWithMethNameViewData, CommonBalanceViewData {


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
    MyVotePresenter presenter = new MyVotePresenter();
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    @BindView(R.id.ll_bgtp)
    LinearLayout ll_bgtp;
    ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean> netList = new ArrayList();

    String zb;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_my_vote;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.my_vote));
        presenter.getVotedProducerList(wallet.getWalletId(), MyWallet.ELA, this);
        tvName.setText(wallet.getWalletName());
        new CommonGetBalancePresenter().getBalance(wallet.getWalletId(), MyWallet.ELA, 2, this);
    }

    @Override
    protected void setExtraData(Bundle data) {
        zb = data.getString("zb");
        super.setExtraData(data);
        netList = (ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean>) data.getSerializable("netList");
    }

    //变更投票
    @OnClick(R.id.ll_bgtp)
    public void onViewClicked() {
        /*List<VoteListBean.DataBean.ResultBean.ProducersBean> newlist = new ArrayList();
        for (int i = 0; i < keylist.size(); i++) {
            for (int j = 0; j < netList.size(); j++) {
                if (netList.get(j).getOwnerpublickey().equals(keylist.get(i))) {
                    newlist.add(netList.get(j));
                }
            }
        }
         CacheDoubleUtils.getInstance().put("list", (Serializable) newlist,  CacheDoubleUtils.DAY * 360);*/
        Bundle bundle = new Bundle();
        bundle.putString("type", "2");
        bundle.putString("zb", zb);
        bundle.putSerializable("netList", (Serializable) netList);
        start(NodeCartFragment.class, bundle);
    }

    Long value = 0L;
    List<String> keylist = new ArrayList();
    //  List<Long> vlauelist = new ArrayList();

    @Override
    public void onGetCommonData(String methodname, String data) {
        switch (methodname) {

            case "getVotedProducerList":
                KLog.a(data);
                if (data.equals("{}")) {
                    ivType.setBackgroundResource(R.mipmap.my_vote_go_img);
                    tvBlank.setVisibility(View.VISIBLE);
                    recyclerview.setVisibility(View.GONE);
                } else {
                    ivType.setBackgroundResource(R.mipmap.found_vote_mine_lock);
                    ll_bgtp.setVisibility(View.VISIBLE);
                    try {
                        JSONObject jsonObject = new JSONObject(data);
                        ll_bgtp.setVisibility(View.VISIBLE);

                        keylist.add(getString(R.string.last_voting_record));

                        Iterator it = jsonObject.keys();

                        while (it.hasNext()) {
                            String key = (String) it.next();
                            keylist.add(key);
                            // value = jsonObject.getLong(key) / MyWallet.RATE + value;
                        }
                        recyclerview.setLayoutManager(new LinearLayoutManager(getContext()));
                        recyclerview.setAdapter(new MyVoteAdapter(keylist,
                                jsonObject, value + ""));
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }
                }

                break;
        }
    }

    @Override
    public void onBalance(BalanceEntity data) {
        KLog.a(data.getBalance());
        //String str = string.substring(0, string.indexOf(".")) + string.substring(string.indexOf(".") + 1);
        //Double num = Double.parseDouble(data.getBalance()) / MyWallet.RATE;
        int num = Arith.div(data.getBalance(), MyWallet.RATE_S).intValue();
        tvNumVote.setText(getString(R.string.right_to_vote_ticket) + num);
    }


    public class MyVoteAdapter extends BaseQuickAdapter<String, BaseViewHolder> {

        String num;
        JSONObject jsonObject;

        public MyVoteAdapter(@Nullable List<String> name, JSONObject jsonObject, String num) {
            super(R.layout.item_myvoteafragment, name);
            this.num = num;
            this.jsonObject = jsonObject;
        }

        @Override
        protected void convert(BaseViewHolder helper, String item) {


            switch (helper.getLayoutPosition()) {

                case 0:
                    helper.setText(R.id.tv_name, item);
                    helper.setText(R.id.tv_no, "");
                    break;
                default:
                    helper.setText(R.id.tv_name, getName(item));
                    try {
                        // helper.setText(R.id.tv_no, Long.parseLong(jsonObject.getString(item)) / MyWallet.RATE + "");
                        //  helper.setText(R.id.tv_no, NumberiUtil.maxNumberFormat((Long.parseLong(jsonObject.getString(item)) / MyWallet.RATE_) + "", 12));
                        helper.setText(R.id.tv_no, NumberiUtil.maxNumberFormat(Arith.div(jsonObject.getString(item), MyWallet.RATE_S), 12));
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }

                    break;
            }
        }
    }

    //获取名字
    private String getName(String name) {

        for (int i = 0; i < netList.size(); i++) {

            if (netList.get(i).getOwnerpublickey().equals(name)) {
                return netList.get(i).getNickname();
            }
        }
        return getString(R.string.unknown);
    }

//    //获取名次
//    private String getNo(String name) {
//
//        for (int i = 0; i < list.size(); i++) {
//            if (list.get(i).getOwnerpublickey().equals(name)) {
//                return i + 1 + "";
//            }
//        }
//        return getString(R.string.unknown);
//    }
}
