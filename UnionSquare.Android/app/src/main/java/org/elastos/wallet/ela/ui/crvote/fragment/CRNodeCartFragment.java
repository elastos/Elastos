package org.elastos.wallet.ela.ui.crvote.fragment;


import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.AppCompatTextView;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.blankj.utilcode.util.KeyboardUtils;
import com.blankj.utilcode.util.ToastUtils;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetBalancePresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.PwdPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.WallletManagePresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.crvote.adapter.CRNodeCartAdapter;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.vote.NodeCart.NodeCartPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VoteActivity;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.klog.KLog;
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
public class CRNodeCartFragment extends BaseFragment implements CommonBalanceViewData, WarmPromptListener, CommmonStringWithMethNameViewData, CRNodeCartAdapter.OnViewClickListener, CRNodeCartAdapter.OnTextChangedListener {

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

    @BindView(R.id.tv_amount)
    AppCompatTextView tvAmount;
    List<CRListBean.DataBean.ResultBean.ProducersBean> list;
    DialogUtil dialogUtil = new DialogUtil();
    @BindView(R.id.ll_blank)
    LinearLayout ll_blank;
    @BindView(R.id.cb_selectall)
    CheckBox cbSelectall;

    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();

    NodeCartPresenter presenter = new NodeCartPresenter();
    PwdPresenter pwdpresenter = new PwdPresenter();
    ArrayList<CRListBean.DataBean.ResultBean.ProducersBean> netList;
    private BigDecimal balance;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_cr_nodecart;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        netList = (ArrayList<CRListBean.DataBean.ResultBean.ProducersBean>) data.getSerializable("netList");
    }

    @Override
    protected void initView(View view) {
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.found_vote_edit);
        tvTitle.setText(getString(R.string.crcvote));
        if (netList == null || netList.size() == 0) {
            //没有来自接口的节点列表数据
            return;
        }
        registReceiver();
        // 为Adapter准备数据
        initDate();
        setRecyclerView(list);
        new CommonGetBalancePresenter().getBalance(wallet.getWalletId(), MyWallet.ELA, 2, this);


    }


    // 初始化数据
    private void initDate() {
        list = CacheUtil.getCRProducerList();
        if (list == null || list.size() == 0) {
            return;
        }
        ArrayList<CRListBean.DataBean.ResultBean.ProducersBean> newlist = new ArrayList<CRListBean.DataBean.ResultBean.ProducersBean>();
        for (CRListBean.DataBean.ResultBean.ProducersBean bean : netList) {
            if (list.contains(bean)) {
                newlist.add(bean);
            }
        }
        Collections.sort(newlist);
        list.clear();
        list.addAll(newlist);
        CacheUtil.setCRProducerList(list);
    }


    public void setRecyclerView(List<CRListBean.DataBean.ResultBean.ProducersBean> list) {

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
            recyclerView.setAdapter(mAdapter);
            mAdapter.setOnViewClickListener(this);
            mAdapter.setOnTextChangedListener(this);
        } else {
            mAdapter.notifyDataSetChanged();
        }
    }


    // 刷新listview和TextView的显示 点击全选或者全不选
    private void dataChanged(int size, boolean statue) {
        //tvAmount.setText(checkNum + getString(R.string.has_been_selected));
        setSelectStatus(size, statue);
        mAdapter.notifyDataSetChanged();
    }

    private void setSelectStatus(int size, boolean statue) {
        for (int i = 0; i < size; i++) {
            //  mAdapter.getDataMap().put(i, statue);
        }
    }


    boolean is = false;//状态值
    List nodelist = new ArrayList();
    JSONArray jsonArray;

    @OnClick({R.id.iv_title_left, R.id.iv_title_right, R.id.tv_delete, R.id.tv_vote, R.id.cb_selectall, R.id.cb_equal})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.cb_equal:
                onClickEqual();
                break;
            case R.id.cb_selectall:
                onClickSelectAll();
                break;
            case R.id.iv_title_left:
                _mActivity.onBackPressed();
                break;
            case R.id.iv_title_right:
                if (is == false) {
                    llBottom2.setVisibility(View.VISIBLE);
                    llBottom1.setVisibility(View.GONE);
                    ivTitleRight.setImageResource(R.mipmap.found_vote_finish);
                    is = true;
                } else {
                    llBottom2.setVisibility(View.GONE);
                    llBottom1.setVisibility(View.VISIBLE);
                    ivTitleRight.setImageResource(R.mipmap.found_vote_edit);
                    is = false;
                }
                break;

            case R.id.tv_delete:
                if (list == null || list.size() == 0) {
                    return;
                }
                List<CRListBean.DataBean.ResultBean.ProducersBean> deleteList = new ArrayList();
               /* for (int i = 0; i < list.size(); i++) {
                    if (mAdapter.getDataMap().get(i)) {
                        deleteList.add(list.get(i));
                    }
                }*/
                list.removeAll(deleteList);
                dataChanged(list.size(), false);
                CacheUtil.setCRProducerList(list);
                break;

            case R.id.tv_vote:
                if (list == null || list.size() == 0) {
                    return;
                }
                nodelist.clear();
                for (int i = 0; i < list.size(); i++) {
                   /* if (mAdapter.getDataMap().get(i)) {
                        nodelist.add(list.get(i).getOwnerpublickey());
                    }*/
                }
                if (nodelist.size() > 36) {
                    showToast(getString(R.string.max36dot));
                    return;
                }


                if (nodelist.size() == 0) {
                    ToastUtils.showShort(getString(R.string.please_select));
                    return;
                }
                jsonArray = JSONArray.parseArray(JSON.toJSONString(nodelist));

                Intent intent = new Intent(getContext(), VoteActivity.class);
                intent.putExtra("balance", "");
                startActivity(intent);
                break;
        }
    }

    //查询余额结果
    @Override
    public void onBalance(BalanceEntity data) {
        balance = Arith.sub(Arith.div(data.getBalance(), MyWallet.RATE_S), "0.01").setScale(8, BigDecimal.ROUND_DOWN);
        tvBalance.setText(getString(R.string.maxvote) + balance.toPlainString());
        tvAvaliable.setText(getString(R.string.available) + balance.toPlainString());
        mAdapter.setBalance(balance);
    }

    String num;

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.VOTETRANSFERACTIVITY.ordinal()) {
            dialogUtil.showWarmPromptInput(getBaseActivity(), null, null, this);
            KeyboardUtils.showSoftInput(getBaseActivity());
            num = result.getName();
        }
    }

    String pwd;

    @Override
    public void affireBtnClick(View view) {
        pwd = ((EditText) view).getText().toString().trim();
        if (TextUtils.isEmpty(pwd)) {
            showToastMessage(getString(R.string.pwdnoempty));
            return;
        }
        new WallletManagePresenter().exportWalletWithMnemonic(wallet.getWalletId(), pwd, this);
    }


    @Override
    public void onGetCommonData(String methodname, String data) {
        //  Double sl = Double.parseDouble(num) * MyWallet.RATE_;
        switch (methodname) {
            //验证密码
            case "exportWalletWithMnemonic":
                //创建投票
                presenter.createVoteProducerTransaction(wallet.getWalletId(), MyWallet.ELA, "",
                        Arith.mul(num, MyWallet.RATE_S).toPlainString(), String.valueOf(jsonArray), "", true, this);
                break;
            //创建投票交易
            case "createVoteProducerTransaction":
                KLog.a("createVoteProducerTransaction" + data);
                //计算手续费
                pwdpresenter.signTransaction(wallet.getWalletId(), MyWallet.ELA, data, pwd, this);
                break;
            case "signTransaction":
                pwdpresenter.publishTransaction(wallet.getWalletId(), MyWallet.ELA, data, this);
                break;
            case "publishTransaction":
                KLog.a(data);
                dialogUtil.dialogDismiss();
                ToastUtils.showShort(getString(R.string.vote_success));
                break;
        }
    }

    private void onClickEqual() {
        mAdapter.initDateStaus(cbEqual.isChecked());
        mAdapter.equalDataMapELA();
        mAdapter.notifyDataSetChanged();
        setOtherUI();
    }

    private void onClickSelectAll() {
        mAdapter.initDateStaus(cbSelectall.isChecked());
        mAdapter.notifyDataSetChanged();
        tvAmount.setText(getString(R.string.totle) + mAdapter.getCountEla().toPlainString() + " ELA");

    }

    @Override
    public void onItemViewClick(CRNodeCartAdapter adapter, View clickView, int position) {
        setSelectAllStatus();
        setOtherUI();
    }

    private void setOtherUI() {
        tvAmount.setText(getString(R.string.totle) + mAdapter.getCountEla().toPlainString() + " ELA");
        BigDecimal countEla = mAdapter.getCountEla();
        if (balance.compareTo(countEla) <= 0) {
            tvAvaliable.setText(getString(R.string.available) + "0 ELA");
        } else {
            tvAvaliable.setText(getString(R.string.available) + balance.subtract(countEla).toPlainString() + " ELA");
        }
    }

    /**
     * 设置下部全选按钮
     */
    private void setSelectAllStatus() {
        int checkSum = mAdapter.getCheckNum();
        if (list == null || list.size() == 0) {
            cbSelectall.setChecked(false);

        } else if (checkSum == list.size()) {
            cbSelectall.setChecked(true);

        } else {
            cbSelectall.setChecked(false);

        }
    }

    @Override
    public void onTextChanged(CRNodeCartAdapter adapter, View clickView, int position) {
        setOtherUI();

    }
}
