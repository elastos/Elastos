package org.elastos.wallet.ela.ui.vote.NodeCart;


import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.AppCompatSeekBar;
import android.support.v7.widget.AppCompatTextView;
import android.text.TextUtils;
import android.view.View;
import android.widget.AdapterView;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.blankj.utilcode.util.KeyboardUtils;
import com.blankj.utilcode.util.ToastUtils;
import com.qmuiteam.qmui.layout.QMUILinearLayout;

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
import org.elastos.wallet.ela.ui.vote.activity.VoteActivity;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.ui.vote.signupfor.SignUpPresenter;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.klog.KLog;
import org.elastos.wallet.ela.utils.listener.NewWarmPromptListener;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 节点购车车
 */
public class NodeCartFragment extends BaseFragment implements CommonBalanceViewData, WarmPromptListener, CommmonStringWithMethNameViewData {


    @BindView(R.id.iv_title_left)
    ImageView ivTitleLeft;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.tv_ratio)
    AppCompatTextView tvRatio;
    @BindView(R.id.tv_blank)
    AppCompatTextView tvBlank;
    @BindView(R.id.tv_sc)
    AppCompatTextView tvSc;
    @BindView(R.id.ll_bj)
    LinearLayout llBj;
    @BindView(R.id.ll_tp)
    QMUILinearLayout llTp;
    @BindView(R.id.listview)
    ListView recyclerView;
    private int checkNum; // 记录选中的条目数量

    private MyAdapter mAdapter;
    @BindView(R.id.checkbox)
    CheckBox checkBox;

    @BindView(R.id.tv_yxz)
    AppCompatTextView tv_yxz;
    @BindView(R.id.tv_num)
    AppCompatTextView tv_num;
    @BindView(R.id.tv_ljtp)
    AppCompatTextView tv_ljtp;
    List<VoteListBean.DataBean.ResultBean.ProducersBean> list;
    DialogUtil dialogUtil = new DialogUtil();
    @BindView(R.id.ll_blank)
    LinearLayout ll_blank;
    @BindView(R.id.sc_checkbox)
    CheckBox sc_checkbox;
    @BindView(R.id.sb_suger)
    AppCompatSeekBar sb_suger;
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();

    NodeCartPresenter presenter = new NodeCartPresenter();
    SignUpPresenter signuppresenter = new SignUpPresenter();
    PwdPresenter pwdpresenter = new PwdPresenter();
    ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean> netList;

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
        tvRatio.setText(NumberiUtil.numberFormat(Double.parseDouble(data.getString("zb", "0")) * 100 + "", 5) + "%");
        sb_suger.setProgress((int) Double.parseDouble(data.getString("zb", "0")) * 100);
        netList = (ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean>) data.getSerializable("netList");
    }

    @Override
    protected void initView(View view) {
        if (netList == null || netList.size() == 0) {
            //没有来着接口的节点列表数据
            return;
        }
        sb_suger.setEnabled(false);
        registReceiver();
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.found_vote_edit);
        tvTitle.setText(mContext.getString(R.string.my_list_candidates));
        // 为Adapter准备数据
        initDate();
        if (list == null || list.size() == 0) {
            ll_blank.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
            tv_num.setText(getString(R.string.futuregenerations) + 0 + ")");
            checkBox.setEnabled(false);
            sc_checkbox.setEnabled(false);
        } else {
            ll_blank.setVisibility(View.GONE);
            recyclerView.setVisibility(View.VISIBLE);
            tv_num.setText(getString(R.string.futuregenerations) + list.size() + ")");
            // 实例化自定义的MyAdapter
            mAdapter = new MyAdapter(list, getContext());
            // 绑定Adapter
            recyclerView.setAdapter(mAdapter);
            // checkBox.setChecked(true);
            checkNum = 0;
            tv_yxz.setText("0" + getString(R.string.has_been_selected));
            checkBox.setEnabled(true);
            sc_checkbox.setEnabled(true);
        }

        sc_checkbox.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (sc_checkbox.isChecked()) {
                    // 遍历list的长度，将MyAdapter中的map值全部设为true
                    // 数量设为list的长度
                    checkNum = list.size();
                    // 刷新listview和TextView的显示
                    dataChanged(list.size(), true);
                } else {
                    checkNum = 0;
                    // 遍历list的长度，将MyAdapter中的map值全部设为true
                    dataChanged(list.size(), false);
                }

            }
        });
        checkBox.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!checkBox.isChecked()) {
                    checkNum = 0;
                    dataChanged(list.size(), false);
                } else if (list.size() <= 36) {
                    if (checkBox.isChecked()) {
                        //全选
                        checkNum = list.size();
                        dataChanged(list.size(), true);
                    }
                } else {
                    //大于36全选
                    dialogUtil.showWarmPrompt2(getBaseActivity(), "", new NewWarmPromptListener() {
                        @Override
                        public void affireBtnClick(View view) {
                            // 遍历list的长度，将MyAdapter中的map值全部设为false
                            setSelectStatus(list.size(), false);
                            checkNum = 36;
                            checkBox.setChecked(false);
                            dataChanged(36, true);
                            dialogUtil.dialogDismiss();
                        }

                        @Override
                        public void onCancel(View view) {
                            checkBox.setChecked(false);
                        }
                    });

                }
            }
        });

        // 绑定listView的监听器
        recyclerView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
                                    long arg3) {
                // 取得ViewHolder对象，这样就省去了通过层层的findViewById去实例化我们需要的cb实例的步骤
                MyAdapter.ViewHolder holder = (MyAdapter.ViewHolder) arg1.getTag();
                // 改变CheckBox的状态
                holder.getCb().toggle();
                // 将CheckBox的选中状况记录下来
                MyAdapter.getIsSelected().put(arg2, holder.getCb().isChecked());
                // 调整选定条目
                if (holder.getCb().isChecked()) {
                    checkNum++;
                } else {
                    checkNum--;
                }
                dataChanged();
            }
        });
    }


    // 初始化数据
    private void initDate() {
        list = CacheUtil.getProducerList();
        if (list == null || list.size() == 0) {
            return;
        }
        ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean> newlist = new ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean>();
        for (VoteListBean.DataBean.ResultBean.ProducersBean bean : netList) {
            if (list.contains(bean)) {
                newlist.add(bean);
            }
        }
        list = newlist;
        CacheUtil.setProducerList(list);
        if (list != null && list.size() != 0) {
            Collections.sort(list);
        }
    }

    // 刷新listview和TextView的显示
    private void dataChanged() {
        // 通知listView刷新
        mAdapter.notifyDataSetChanged();
        // TextView显示最新的选中数目
        tv_yxz.setText(checkNum + getString(R.string.has_been_selected));
        setCheckBox(list);
    }

    private void setCheckBox(List list) {
        List selectlist = new ArrayList();
        for (int i = 0; i < list.size(); i++) {
            if (MyAdapter.getIsSelected().get(i)) {
                selectlist.add(i);
            }
        }
        if (list.size() == selectlist.size()) {
            checkBox.setChecked(true);
            sc_checkbox.setChecked(true);
        } else {
            checkBox.setChecked(false);
            sc_checkbox.setChecked(false);
        }
    }

    // 刷新listview和TextView的显示 点击全选或者全不选
    private void dataChanged(int size, boolean statue) {
        tv_yxz.setText(checkNum + getString(R.string.has_been_selected));
        setSelectStatus(size, statue);
        mAdapter.notifyDataSetChanged();
    }

    private void setSelectStatus(int size, boolean statue) {
        for (int i = 0; i < size; i++) {
            MyAdapter.getIsSelected().put(i, statue);
        }
    }


    public static NodeCartFragment newInstance() {
        Bundle args = new Bundle();
        NodeCartFragment fragment = new NodeCartFragment();
        fragment.setArguments(args);
        return fragment;
    }

    boolean is = false;//状态值
    List nodelist = new ArrayList();
    JSONArray jsonArray;

    @OnClick({R.id.iv_title_left, R.id.iv_title_right, R.id.tv_sc, R.id.tv_ljtp})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_title_left:
                _mActivity.onBackPressed();
                break;
            case R.id.iv_title_right:
                if (is == false) {
                    llBj.setVisibility(View.VISIBLE);
                    llTp.setVisibility(View.GONE);
                    ivTitleRight.setImageResource(R.mipmap.found_vote_finish);
                    is = true;
                } else {
                    llBj.setVisibility(View.GONE);
                    llTp.setVisibility(View.VISIBLE);
                    ivTitleRight.setImageResource(R.mipmap.found_vote_edit);
                    is = false;
                }
                break;

            case R.id.tv_sc:
                if (list == null || list.size() == 0) {
                    return;
                }
                List<VoteListBean.DataBean.ResultBean.ProducersBean> deleteList = new ArrayList();
                for (int i = 0; i < list.size(); i++) {
                    if (MyAdapter.getIsSelected().get(i)) {
                        deleteList.add(list.get(i));
                    }
                }
                checkNum = checkNum - deleteList.size();
                list.removeAll(deleteList);
                dataChanged(list.size(), false);
                tv_num.setText(getString(R.string.futuregenerations) + list.size() + ")");
                CacheUtil.setProducerList(list);
                break;

            case R.id.tv_ljtp:
                if (list == null || list.size() == 0) {
                    return;
                }
                nodelist.clear();
                for (int i = 0; i < list.size(); i++) {
                    if (MyAdapter.getIsSelected().get(i)) {
                        nodelist.add(list.get(i).getOwnerpublickey());
                    }
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

                new CommonGetBalancePresenter().getBalance(wallet.getWalletId(), MyWallet.ELA, 2, this);
                break;
        }
    }

    //查询余额结果
    @Override
    public void onBalance(BalanceEntity data) {
        Intent intent = new Intent(getContext(), VoteActivity.class);
        KLog.a(data.getBalance());
        intent.putExtra("fee", data.getBalance());
        startActivity(intent);
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
//                list.removeAll(nodelistbean);
//                 CacheDoubleUtils.getInstance().put("list", (Serializable) list,  CacheDoubleUtils.DAY * 360);
//                checkNum = list.size();
//                tv_num.setText(getString(R.string.futuregenerations) + list.size() + ")");
                // dataChanged();
//                if (list.size() == 0) {
//                    ll_blank.setVisibility(View.VISIBLE);
//                    recyclerView.setVisibility(View.GONE);
//                }
                break;
        }
    }


}
