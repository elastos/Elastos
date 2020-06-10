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

package org.elastos.wallet.ela.ui.vote.NodeCart;


import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.AppCompatSeekBar;
import android.support.v7.widget.AppCompatTextView;
import android.text.TextUtils;
import android.view.View;
import android.widget.AdapterView;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
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
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRlistPresenter;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalSearchEntity;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalDetailPresenter;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VoteActivity;
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

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 节点购车车
 */
public class NodeCartFragment extends BaseFragment implements CommonBalanceViewData, NewBaseViewData, AdapterView.OnItemClickListener {


    @BindView(R.id.iv_title_left)
    ImageView ivTitleLeft;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.tv_ratio)
    AppCompatTextView tvRatio;
    @BindView(R.id.listview)
    ListView recyclerView;
    @BindView(R.id.line_select)
    View lineSelect;
    @BindView(R.id.tv_select)
    TextView tvSelect;
    @BindView(R.id.ll_select)
    LinearLayout llSelect;
    @BindView(R.id.line_unselect)
    View lineUnselect;
    @BindView(R.id.tv_unselect)
    TextView tvUnselect;
    @BindView(R.id.ll_unselect)
    LinearLayout llUnselect;
    @BindView(R.id.ll_tab)
    LinearLayout llTab;
    @BindView(R.id.ll_rate)
    RelativeLayout llRate;
    private int checkNum = 0; // 记录选中的条目数量

    private MyAdapter mAdapter;
    private MyAdapter selectAdapter;
    private MyAdapter unSelectAdapter;

    @BindView(R.id.checkbox)
    CheckBox checkBox;

    @BindView(R.id.tv_yxz)
    AppCompatTextView tv_yxz;
    @BindView(R.id.tv_num)
    AppCompatTextView tv_num;
    @BindView(R.id.tv_ljtp)
    AppCompatTextView tv_ljtp;
    List<VoteListBean.DataBean.ResultBean.ProducersBean> list = new ArrayList<>();
    List<VoteListBean.DataBean.ResultBean.ProducersBean> unSelectlist;//未选择
    DialogUtil dialogUtil = new DialogUtil();
    @BindView(R.id.ll_blank)
    LinearLayout ll_blank;
    @BindView(R.id.sb_suger)
    AppCompatSeekBar sb_suger;
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();

    NodeCartPresenter presenter = new NodeCartPresenter();
    ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean> netList;
    int curentPage = 0;//0 首页 1已选择 2未选择
    private String maxBalance;

    private List<ProposalSearchEntity.DataBean.ListBean> searchBeanList;
    private List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> crList;
    private int transType = 1001;
    private ProposalDetailPresenter proposalDetailPresenter;
    private org.json.JSONArray otherUnActiveVote;
    private List<CtListBean.Council> councilList;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_node_cart;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        tvRatio.setText(NumberiUtil.numberFormat(Double.parseDouble(data.getString("zb", "0")) * 100 + "", 2) + "%");
        sb_suger.setProgress((int) (Double.parseDouble(data.getString("zb", "0")) * 100));
        //netList后期还承担未选择list
        netList = (ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean>) data.getSerializable("netList");
        new ProposalPresenter().proposalSearch(-1, -1, "ALL", null, this);
        new CRlistPresenter().getCRlist(-1, -1, "all", this, true);
        new CtListPresenter().getCouncilList(this, String.valueOf(1));

    }

    @Override
    protected void initView(View view) {
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.found_vote_edit);
        tvTitle.setText(mContext.getString(R.string.my_list_candidates));
        if (netList == null) {
            //没有来着接口的节点列表数据
            netList = new ArrayList<>();
        }
        registReceiver();
        // 为Adapter准备数据
        initDate();
        // 绑定listView的监听器
        tv_num.setText(getString(R.string.future_generations) + "(" + list.size() + ")");//全选
        setRecyclerView(mAdapter, list);
        //这里list已经排序
        CacheUtil.setProducerList(list);
        recyclerView.setOnItemClickListener(this);
    }

    /**
     * 设置下部全选按钮
     *
     * @param mAdapter
     */
    private void setSelectAllStatus(MyAdapter mAdapter) {
        int checkSum = mAdapter.getCheckNum();
        tv_yxz.setText(checkSum + getString(R.string.has_been_selected));
        if (mAdapter.getList() == null || mAdapter.getList().size() == 0) {
            checkBox.setChecked(false);

        } else if (checkSum == mAdapter.getList().size()) {
            checkBox.setChecked(true);

        } else {
            checkBox.setChecked(false);

        }
    }

    public void setRecyclerView(MyAdapter mAdapter, List<VoteListBean.DataBean.ResultBean.ProducersBean> list) {
        Collections.sort(list);
        if (list == null || list.size() == 0) {
            ll_blank.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
        } else {
            ll_blank.setVisibility(View.GONE);
            recyclerView.setVisibility(View.VISIBLE);
        }
        if (mAdapter == null) {
            mAdapter = new MyAdapter(list, this);
            if (curentPage == 0) {
                this.mAdapter = mAdapter;
            } else if (curentPage == 1) {
                this.selectAdapter = mAdapter;
            } else if (curentPage == 2) {
                this.unSelectAdapter = mAdapter;
            }
        } else {

            mAdapter.setList(list);
        }
        recyclerView.setAdapter(mAdapter);//一个rv  多个adpter  这里用来切换adapter  不能notifydatachange'

        setSelectAllStatus(mAdapter);

    }

    // 初始化数据
    private void initDate() {
        ArrayList<String> list1 = CacheUtil.getProducerListString();
        // list = CacheUtil.getProducerList();
        unSelectlist = new ArrayList<>();
        ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean> newlist = new ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean>();
        for (VoteListBean.DataBean.ResultBean.ProducersBean bean : netList) {
            //刷新本地数据
            if (list1.contains(bean.getOwnerpublickey())) {
                newlist.add(bean);
            }
        }
        unSelectlist.addAll(netList);
        unSelectlist.removeAll(newlist);//netList变成了未选择
        //刷新list数据
        list.clear();
        list.addAll(newlist);
    }


    boolean is = false;//状态值
    List nodelist;


    @OnClick({R.id.iv_title_left, R.id.iv_title_right, R.id.tv_ljtp, R.id.ll_select, R.id.ll_unselect, R.id.checkbox})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.ll_select:
                //已经选择的列表
                clickSelectList();
                break;
            case R.id.ll_unselect:
                //没有选择的列表
                clickUnSelectList();
                break;
            case R.id.iv_title_left:
                _mActivity.onBackPressed();
                break;
            case R.id.iv_title_right:
                if (!is) {
                    //显示点击右上角  相当于默认点击一次已选列表
                    llRate.setVisibility(View.GONE);
                    ivTitleRight.setImageResource(R.mipmap.found_vote_finish);
                    is = true;
                    llTab.setVisibility(View.VISIBLE);
                    clickSelectList();

                } else {
                    tv_ljtp.setText(getString(R.string.mmediately_to_vote));
                    curentPage = 0;
                    //显示再次点击右上角
                    llRate.setVisibility(View.VISIBLE);
                    ivTitleRight.setImageResource(R.mipmap.found_vote_edit);
                    is = false;
                    llTab.setVisibility(View.GONE);
                    tv_num.setText(getString(R.string.future_generations) + "(" + list.size() + ")");//全选
                    setRecyclerView(mAdapter, list);
                }
                break;


            case R.id.tv_ljtp:
                //立即投票 删除 添加
                switch (curentPage) {
                    case 0:
                        if (proposalDetailPresenter == null) {
                            proposalDetailPresenter = new ProposalDetailPresenter();
                        }
                        proposalDetailPresenter.getVoteInfo(wallet.getWalletId(), "", this);

                        break;

                    case 1:
                        doDelete();
                        break;
                    case 2:
                        doAdd();
                        break;
                }
                break;
            case R.id.checkbox:
                //全选
                onClickSelectAll();
                break;

        }
    }

    private void doVote() {
        if (list == null || list.size() == 0) {
            return;
        }
        if (nodelist == null) {
            nodelist = new ArrayList();
        } else {
            nodelist.clear();
        }
        for (int i = 0; i < list.size(); i++) {
            if (mAdapter.getDataMap().get(i)) {
                nodelist.add(list.get(i).getOwnerpublickey());
            }
        }
        //  nodelist.add("03d835419df96c76b4b7bd5e56b1ed9362c80186ce3b35cd962dec2232040b0f8m");
        if (nodelist.size() > 36) {
            showToast(getString(R.string.max36dot));
            return;
        }


        if (nodelist.size() == 0) {
            ToastUtils.showShort(getString(R.string.please_select));
            return;
        }

        new CommonGetBalancePresenter().getBalance(wallet.getWalletId(), MyWallet.ELA, 2, this);

    }

    private void doDelete() {
        if (list == null || list.size() == 0) {
            return;
        }
        MyAdapter curentAdapter = ((MyAdapter) recyclerView.getAdapter());
        List<VoteListBean.DataBean.ResultBean.ProducersBean> deleteList = curentAdapter.getAllSelectList();
        checkNum = checkNum - deleteList.size();
        list.removeAll(deleteList);
        unSelectlist.addAll(deleteList);
        tv_yxz.setText(0 + getString(R.string.has_been_selected));
        InitAllAdapterDataMap();
        Collections.sort(list);
        curentAdapter.setList(list);
        curentAdapter.notifyDataSetChanged();
        tv_num.setText(getString(R.string.future_generations) + "(" + list.size() + ")");
        tv_yxz.setText("0" + getString(R.string.has_been_selected));
        CacheUtil.setProducerList(list);

    }


    private void doAdd() {
        MyAdapter curentAdapter = ((MyAdapter) recyclerView.getAdapter());
        List<VoteListBean.DataBean.ResultBean.ProducersBean> addList = curentAdapter.getAllSelectList();
        list.addAll(addList);
        unSelectlist.removeAll(addList);
        InitAllAdapterDataMap();
        Collections.sort(list);
        curentAdapter.setList(unSelectlist);
        curentAdapter.notifyDataSetChanged();
        tv_yxz.setText("0" + getString(R.string.has_been_selected));
        tv_num.setText(getString(R.string.future_generations) + "(" + unSelectlist.size() + ")");
        CacheUtil.setProducerList(list);
    }

    private void InitAllAdapterDataMap() {
        mAdapter.initDateStaus(false);
        selectAdapter.initDateStaus(false);
        if (unSelectAdapter != null) {
            unSelectAdapter.initDateStaus(false);
        }
    }

    private void clickUnSelectList() {
        tv_ljtp.setText(getString(R.string.add));
        curentPage = 2;
        lineSelect.setVisibility(View.GONE);
        lineUnselect.setVisibility(View.VISIBLE);
        tvSelect.setTextColor(getResources().getColor(R.color.whiter50));
        tvUnselect.setTextColor(getResources().getColor(R.color.whiter));
        tv_num.setText(getString(R.string.future_generations) + "(" + unSelectlist.size() + ")");//全选
        setRecyclerView(unSelectAdapter, unSelectlist);
    }

    private void clickSelectList() {
        tv_ljtp.setText(getString(R.string.delete));
        curentPage = 1;
        lineSelect.setVisibility(View.VISIBLE);
        lineUnselect.setVisibility(View.GONE);
        tvSelect.setTextColor(getResources().getColor(R.color.whiter));
        tvUnselect.setTextColor(getResources().getColor(R.color.whiter50));
        tv_num.setText(getString(R.string.future_generations) + "(" + list.size() + ")");//全选
        setRecyclerView(selectAdapter, list);
    }


    //查询余额结果
    @Override
    public void onBalance(BalanceEntity data) {
        Intent intent = new Intent(getContext(), VoteActivity.class);
        BigDecimal balance = Arith.div(Arith.sub(data.getBalance(), 1000000), MyWallet.RATE_S, 8);
        maxBalance = NumberiUtil.removeZero(balance.toPlainString());
        //小于1 huo 0
        if ((balance.compareTo(new BigDecimal(0)) <= 0)) {
            maxBalance = "0";
            intent.putExtra("maxBalance", "0");
        } else if ((balance.compareTo(new BigDecimal(1)) < 0)) {
            intent.putExtra("maxBalance", "< 1");
        } else {
            intent.putExtra("maxBalance", maxBalance);
        }
        intent.putExtra("type", Constant.SUPERNODEVOTE);
        intent.putExtra("openType", this.getClass().getSimpleName());
        startActivity(intent);
    }

    String num;//投票数量

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.VOTETRANSFERACTIVITY.ordinal() && getClass().getSimpleName().equals(result.getName())) {
            num = (String) result.getObj();
            String amount;
            if ("MAX".equals(num)) {
                amount = Arith.mulRemoveZero(maxBalance, MyWallet.RATE_S).toPlainString();
            } else {
                amount = Arith.mulRemoveZero(num, MyWallet.RATE_S).toPlainString();
            }
            presenter.createVoteProducerTransaction(wallet.getWalletId(), MyWallet.ELA, "",
                    amount, String.valueOf(JSONArray.parseArray(JSON.toJSONString(nodelist))), "", otherUnActiveVote.toString(), this);

        }
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


    private void showOpenDraftWarm(String attributesJson) {
        new DialogUtil().showCommonWarmPrompt(getBaseActivity(), getString(R.string.notsufficientfundskeepornot),
                getString(R.string.sure), getString(R.string.cancel), false, new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        goTransferActivity(attributesJson);
                    }
                });
    }

    private void goTransferActivity(String attributesJson) {
        Intent intent = new Intent(getActivity(), TransferActivity.class);
        intent.putExtra("amount", num);
        intent.putExtra("wallet", wallet);
        intent.putExtra("chainId", MyWallet.ELA);
        intent.putExtra("attributes", attributesJson);
        intent.putExtra("type", Constant.SUPERNODEVOTE);
        intent.putExtra("transType", transType);
        startActivity(intent);
    }

    //点击全选按钮
    private void onClickSelectAll() {
        MyAdapter curentAdapter = ((MyAdapter) recyclerView.getAdapter());
        if (!checkBox.isChecked()) {
            curentAdapter.initDateStaus(false);
        } else if (curentPage == 0 && curentAdapter.getList().size() > 36) {
            //大于36全选
            dialogUtil.showWarmPrompt2(getBaseActivity(), "", new NewWarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                    // 遍历list的长度，将MyAdapter中的map值全部设为false
                    curentAdapter.initDateStaus(false);
                    checkBox.setChecked(false);
                    curentAdapter.setDateStaus(36, true);
                    curentAdapter.notifyDataSetChanged();
                    tv_yxz.setText("36" + getString(R.string.has_been_selected));
                    dialogUtil.dialogDismiss();
                }

                @Override
                public void onCancel(View view) {
                    checkBox.setChecked(false);
                }
            });

        } else {
            curentAdapter.initDateStaus(true);
        }
        setSelectAllStatus(curentAdapter);
        curentAdapter.notifyDataSetChanged();

    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
                            long arg3) {
        // 取得ViewHolder对象，这样就省去了通过层层的findViewById去实例化我们需要的cb实例的步骤
        MyAdapter.ViewHolder holder = (MyAdapter.ViewHolder) arg1.getTag();
        // 改变CheckBox的状态
        holder.getCb().toggle();
        // 将CheckBox的选中状况记录下来
        ((MyAdapter) arg0.getAdapter()).getDataMap().put(arg2, holder.getCb().isChecked());

        setSelectAllStatus(((MyAdapter) arg0.getAdapter()));


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
            case "getCRlist":
                crList = ((CRListBean) baseEntity).getData().getResult().getCrcandidatesinfo();
                break;
            case "getVoteInfo":
                //剔除非公示期的
                String voteInfo = ((CommmonStringEntity) baseEntity).getData();
                otherUnActiveVote = proposalDetailPresenter.conversUnactiveVote("Delegate", voteInfo, netList, crList, searchBeanList, councilList);
                doVote();
                break;

            case "createVoteProducerTransaction":
                //创建投票交易
                String attributes = ((CommmonStringEntity) baseEntity).getData();
                JSONObject attributesJson = JSON.parseObject(attributes);
                String status = attributesJson.getString("DropVotes");
                if (!TextUtils.isEmpty(status) && !status.equals("[]")) {
                    transType = 1002;
                    showOpenDraftWarm(attributes);
                    break;
                }
                transType = 1001;
                goTransferActivity(attributes);
                break;

        }
    }

}
