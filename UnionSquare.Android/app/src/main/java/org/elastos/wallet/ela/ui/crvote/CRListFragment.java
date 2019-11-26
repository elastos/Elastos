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
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.chad.library.adapter.base.BaseQuickAdapter;
import com.qmuiteam.qmui.layout.QMUILinearLayout;
import com.scwang.smartrefresh.layout.SmartRefreshLayout;
import com.scwang.smartrefresh.layout.api.RefreshLayout;
import com.scwang.smartrefresh.layout.listener.OnLoadMoreListener;
import com.scwang.smartrefresh.layout.listener.OnRefreshListener;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.fragment.AddAssetFragment;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.bean.ISubWalletListEntity;
import org.elastos.wallet.ela.ui.crvote.adapter.CRListAdapter;
import org.elastos.wallet.ela.ui.crvote.adapter.CRListAdapter1;
import org.elastos.wallet.ela.ui.crvote.adapter.CRListAdapterFather;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.fragment.CRAgreementFragment;
import org.elastos.wallet.ela.ui.crvote.fragment.CRInformationFragment;
import org.elastos.wallet.ela.ui.crvote.fragment.CRManageFragment;
import org.elastos.wallet.ela.ui.crvote.fragment.CRMyVoteFragment;
import org.elastos.wallet.ela.ui.crvote.fragment.CRNodeCartFragment;
import org.elastos.wallet.ela.ui.crvote.fragment.CRSignUpForFragment;
import org.elastos.wallet.ela.ui.crvote.presenter.CRlistPresenter;
import org.elastos.wallet.ela.ui.did.entity.AllPkEntity;
import org.elastos.wallet.ela.ui.did.presenter.AddDIDPresenter;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.DividerItemDecoration;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.listener.NewWarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 超级节点选举
 */
public class CRListFragment extends BaseFragment implements BaseQuickAdapter.OnItemClickListener, OnRefreshListener, NewBaseViewData, OnLoadMoreListener {
    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.recyclerview)
    RecyclerView recyclerview;
    ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> netList;
    @BindView(R.id.iv_swichlist)
    ImageView ivSwichlist;
    @BindView(R.id.iv_toselect)
    ImageView ivToSelect;
    @BindView(R.id.recyclerview1)
    RecyclerView recyclerview1;

    @BindView(R.id.tv_signupfor)
    TextView tv_signupfor;
    @BindView(R.id.srl)
    SmartRefreshLayout srl;
    @BindView(R.id.ll_bottom1)
    QMUILinearLayout llBottom1;
    @BindView(R.id.ll_bottom2)
    LinearLayout llBottom2;
    @BindView(R.id.cb_selectall)
    CheckBox cbSelectall;
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    CRlistPresenter presenter;
    private CRListAdapter1 adapter1;
    private CRListAdapter adapter;
    private CRListAdapterFather curentAdapter;
    private CRListBean.DataBean.ResultBean.CrcandidatesinfoBean curentNode;
    private String did;
    private int pageNum = 1;
    private final int pageSize = 1000;//基本没分页了
    private AddDIDPresenter addDIDPresenter;
    private boolean click;//点击创建或者管理cr  click=true

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_cr_list;
    }

    @Override
    protected void initView(View view) {

        setToobar(toolbar, toolbarTitle, getString(R.string.crcvote), getString(R.string.voting_rules));
        presenter = new CRlistPresenter();
        //presenter.getCROwnerPublicKey(wallet.getWalletId(), MyWallet.ELA, this);
        //获取公钥
        srl.setOnRefreshListener(this);
        srl.setOnLoadMoreListener(this);

        //获取选举状态
        presenter.getRegisteredCRInfo(wallet.getWalletId(), MyWallet.ELA, this);
        addDIDPresenter = new AddDIDPresenter();
        registReceiver();
    }

    @OnClick({R.id.tv_myvote, R.id.tv_title_right, R.id.tv_going_to_vote, R.id.tv_signupfor, R.id.iv_swichlist, R.id.iv_toselect, R.id.ll_add, R.id.cb_selectall})
    public void onViewClicked(View view) {
        Bundle bundle;
        switch (view.getId()) {
            case R.id.ll_add:
                //批量加入
                boolean tag = false;
                ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list = CacheUtil.getCRProducerList();
                for (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean : netList) {
                    //存储选中的=原来的(isSelect)+getChecckPosition
                    if (bean.isChecked()) {
                        tag = true;
                        list.add(bean);
                    }
                }
                if (tag) {
                    CacheUtil.setCRProducerList(list);
                    showToast(getString(R.string.addsucess));
                }

                //关闭批量加入购物车状态
                closeAdd();
                break;
            case R.id.cb_selectall:

                //全选
                if (((CheckBox) view).isChecked()) {
                    curentAdapter.addAllPositionAndNotify();
                } else {
                    curentAdapter.removeAllPositionAndNotify();
                }

                break;
            case R.id.tv_myvote:
                bundle = new Bundle();
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
                bundle.putSerializable("netList", netList);
                start(CRNodeCartFragment.class, bundle);
                break;
            case R.id.tv_signupfor:
                bundle = new Bundle();
                bundle.putString("did", did);
                if (status.equals("Unregistered")) {
                    start(CRAgreementFragment.class);

                } else {
                    if (did != null) {
                        bundle.putString("status", status);
                        bundle.putString("info", info);
                        bundle.putParcelable("curentNode", curentNode);
                        start(CRManageFragment.class, bundle);
                        return;
                    }
                    click = true;
                    addDIDPresenter.getAllSubWallets(wallet.getWalletId(), this);
                }

                break;
            case R.id.iv_swichlist:
                //两种list切换展示
                if (recyclerview.getVisibility() == View.VISIBLE) {
                    ivSwichlist.setImageResource(R.mipmap.vote_switch_squeral);
                    recyclerview.setVisibility(View.GONE);
                    recyclerview1.setVisibility(View.VISIBLE);
                    curentAdapter = adapter1;
                } else {
                    ivSwichlist.setImageResource(R.mipmap.vote_switch_list);
                    recyclerview1.setVisibility(View.GONE);
                    recyclerview.setVisibility(View.VISIBLE);
                    curentAdapter = adapter;
                }
                break;
            case R.id.iv_toselect:
                //批量加入购物车 当展示这种页面ivSwichlist会隐藏
                if (ivSwichlist.getVisibility() == View.VISIBLE) {
                    ivToSelect.setImageResource(R.mipmap.found_vote_finish);
                    ivSwichlist.setVisibility(View.GONE);
                    llBottom2.setVisibility(View.VISIBLE);
                    llBottom1.setVisibility(View.GONE);
                    //同步已经加入购物车的数据setSelect
                    ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list1 = CacheUtil.getCRProducerList();
                    if (list1 != null && list1.size() > 0) {
                        for (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean : netList) {
                            if (list1.contains(bean)) {
                                bean.setSelect(true);
                            }
                        }
                    }
                    curentAdapter.setShowCheckbox(true);

                } else {
                    closeAdd();
                }


                break;
        }
    }

    private void closeAdd() {
        ivToSelect.setImageResource(R.mipmap.multi_import_btn);
        ivSwichlist.setVisibility(View.VISIBLE);
        llBottom2.setVisibility(View.GONE);
        llBottom1.setVisibility(View.VISIBLE);
        curentAdapter.setShowCheckbox(false);
       /* curentAdapter.removeAllPosition();
        if (cbSelectall.isChecked()) {
            cbSelectall.setChecked(false);
        }*/

    }


    @Override
    public void onItemClick(BaseQuickAdapter adapter, View view, int position) {
        if (((CRListAdapterFather) adapter).isShowCheckbox()) {
            CheckBox cb = view.findViewById(R.id.checkbox);
            (cb).toggle();
            netList.get(position).setChecked(cb.isChecked());
            /*if (cb.isChecked()) {
                ((CRListAdapterFather) adapter).getChecckPosition().add(position);
            } else {
                ((CRListAdapterFather) adapter).getChecckPosition().remove(position);
            }*/

            //adapter.notifyDataSetChanged();优化内存  不用这个
            return;
        }

        Bundle bundle = new Bundle();
        bundle.putInt("postion", position);
        bundle.putSerializable("netList", netList);

        start(CRInformationFragment.class, bundle);
    }


    boolean is = false;//是否有自已的选举


    private void setRecyclerview() {
        if (adapter == null) {
            recyclerview.setLayoutManager(new GridLayoutManager(getContext(), 2));
            DividerItemDecoration decoration = new DividerItemDecoration(getActivity(), DividerItemDecoration.BOTH_SET, 10, R.color.transparent);
            recyclerview.addItemDecoration(decoration);
            adapter = new CRListAdapter(this, netList, is);
            adapter.setPos(pos);
            adapter.setOnItemClickListener(this);
            recyclerview.setAdapter(adapter);
            if (curentAdapter == null)
                curentAdapter = adapter;
        } else {
            adapter.setIs(is);
            adapter.setPos(pos);
            adapter.notifyDataSetChanged();
        }

    }

    private void setRecyclerview1() {
        if (adapter1 == null) {
            recyclerview1.setLayoutManager(new LinearLayoutManager(getContext()));
            adapter1 = new CRListAdapter1(this, netList, is);
            adapter1.setOnItemClickListener(this);
            adapter1.setPos(pos);
            recyclerview1.setAdapter(adapter1);
        } else {
            adapter1.setIs(is);
            adapter1.setPos(pos);
            adapter1.notifyDataSetChanged();
        }
    }


    public void onGetVoteList(List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> data) {
        if (netList == null) {
            netList = new ArrayList<>();
        }

        if (pageNum == 1) {
            netList.clear();
        } else if (data == null || data.size() == 0) {
            showToastMessage(getString(R.string.loadall));
            return;
        }
        if (data != null && data.size() != 0) {
            netList.addAll(data);
            //pos==-1表示未移除过 先移除  并获得移除的位置  待添加
            if (curentNode != null && pos == -1) {
                pos = netList.indexOf(curentNode);
                netList.remove(curentNode);
            }
            //判断是否需要添加到首位
            if (!is && curentNode != null && status.equals("Registered")) {
                netList.add(0, curentNode);
                is = true;
            }
        }

        setRecyclerview();
        setRecyclerview1();

        pageNum++;
    }

    int pos = -1;

    /**
     * 重置信息  获得当前节点详情  剔除非active数据
     *
     * @param list
     * @param totalvotes
     */
    private void resetData(List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list, String totalvotes) {

        Iterator<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> iterator = list.iterator();
        while (iterator.hasNext()) {
            //筛选当前节点
            CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean = iterator.next();
            if (curentNode == null) {
                if (bean.getDid().equals(did)) {
                    curentNode = bean;
                }
            }
            //删除非active节点
            if (!bean.getState().equals("Active")) {
                iterator.remove();//date  remove 不影响netlist  date修改影响netlist
                continue;
            }
            setVoterate(bean, totalvotes);

        }


    }

    private String status;
    private String info;


    private Drawable getDrawable(int id) {
        Drawable drawable = getResources().getDrawable(id);
        drawable.setBounds(0, 0, drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight());
        return drawable;
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

        switch (methodName) {
            case "getAllSubWallets":
                ISubWalletListEntity subWalletListEntity = (ISubWalletListEntity) baseEntity;
                for (SubWallet subWallet : subWalletListEntity.getData()) {
                    if (subWallet.getChainId().equals(MyWallet.IDChain)) {
                        addDIDPresenter.getAllPublicKeys(wallet.getWalletId(), MyWallet.IDChain, 0, 1, CRListFragment.this);
                        return;
                    }
                }
                //没有对应的子钱包 需要打开idchain
                showOpenDIDWarm(subWalletListEntity);

                break;

            case "getAllPublicKeys":
                AllPkEntity allPkEntity = JSON.parseObject(((CommmonStringEntity) baseEntity).getData(), AllPkEntity.class);

                if (allPkEntity.getPublicKeys() == null || allPkEntity.getPublicKeys().size() == 0) {
                    return;
                }
                addDIDPresenter.getDIDByPublicKey(wallet.getWalletId(), allPkEntity.getPublicKeys().get(0), this);
                break;
            case "getDIDByPublicKey":
                did = ((CommmonStringEntity) baseEntity).getData();
                //无论是点击创建或者管理  还是打开页面就加载的  did肯定都为null 不为null的上层已经拦截    两种情况互斥
                pageNum = 1;
                if (status.equals("Unregistered") && click) {
                    //Unregistered刚进入时候不判断是否有idchain直接加载activelist
                    click = false;
                    Bundle bundle = new Bundle();
                    bundle.putString("did", did);
                    bundle.putSerializable("netList", netList);
                    start(CRSignUpForFragment.class, bundle);
                } else {
                    //如何是点击需要刷新全部数据  如果是刚打来 获得数据
                    presenter.getCRlist(pageNum, pageSize, "all", this);
                }
                //如果是是点击创建或者管理  跳转相应页面

                break;
            case "getActiveCRlist":
                List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> curentList = ((CRListBean) baseEntity).getData().getResult().getCrcandidatesinfo();

                try {
                    String totalvotes = ((CRListBean) baseEntity).getData().getResult().getTotalvotes();
                    Iterator<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> iterator = curentList.iterator();
                    while (iterator.hasNext()) {
                        //筛选当前节点
                        CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean = iterator.next();
                        setVoterate(bean, totalvotes);
                    }
                    onGetVoteList(curentList);


                } catch (Exception e) {

                    onGetVoteList(null);
                }
                break;
            case "getCRlist":
                //非unregister并且打开了idchain才会到这来  获得所有cr
                List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> curentAllList = ((CRListBean) baseEntity).getData().getResult().getCrcandidatesinfo();
                try {
                    String totalvotes = ((CRListBean) baseEntity).getData().getResult().getTotalvotes();
                    //重置信息  获得当前节点详情  剔除非active数据
                    resetData(curentAllList, totalvotes);

                    onGetVoteList(curentAllList);
                    if (click) {
                        click = false;
                        Bundle bundle = new Bundle();
                        bundle.putString("did", did);

                        if (status.equals("Unregistered")) {
                            bundle.putSerializable("netList", netList);
                            start(CRSignUpForFragment.class, bundle);
                        } else {
                            bundle.putString("status", status);
                            bundle.putString("info", info);
                            bundle.putParcelable("curentNode", curentNode);

                            start(CRManageFragment.class, bundle);
                        }
                    }
                } catch (Exception e) {
                    onGetVoteList(null);
                }


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
                            presenter.getActiveCRlist(pageNum, pageSize, this);
                            break;
                        case "ReturnDeposit":
                            tv_signupfor.setVisibility(View.GONE);
                            addDIDPresenter.getAllSubWallets(wallet.getWalletId(), this);

                            break;
                        case "Canceled":
                        case "Registered":
                            tv_signupfor.setText(getString(R.string.electoral_affairs));
                            tv_signupfor.setVisibility(View.VISIBLE);
                            tv_signupfor.setCompoundDrawables(null, getDrawable(R.mipmap.vote_management), null, null);
                            addDIDPresenter.getAllSubWallets(wallet.getWalletId(), this);

                            break;

                    }
                }
                break;
        }
    }

    @Override
    public void onRefresh(RefreshLayout refreshLayout) {
        onErrorRefreshLayout(srl);
        pageNum = 1;
        is = false;
        presenter.getRegisteredCRInfo(wallet.getWalletId(), MyWallet.ELA, this);
    }

    @Override
    public void onLoadMore(RefreshLayout refreshLayout) {
        onErrorRefreshLayout(srl);
        if (did == null) {
            presenter.getActiveCRlist(pageNum, pageSize, CRListFragment.this);
        } else {
            presenter.getCRlist(pageNum, pageSize, "all", this);
        }

    }

    private void showOpenDIDWarm(ISubWalletListEntity subWalletListEntity) {
        new DialogUtil().showCommonWarmPrompt1(getBaseActivity(), getString(R.string.noidchainopenornot),
                getString(R.string.toopen), getString(R.string.cancel), false, new NewWarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        Bundle bundle = new Bundle();
                        bundle.putString("walletId", wallet.getWalletId());
                        ArrayList<String> chainIds = new ArrayList<>();
                        for (SubWallet iSubWallet : subWalletListEntity.getData()) {
                            chainIds.add(iSubWallet.getChainId());
                        }
                        bundle.putStringArrayList("chainIds", chainIds);
                        start(AddAssetFragment.class, bundle);
                    }

                    @Override
                    public void onCancel(View view) {
                        if (netList == null)//代表没请求过数据   为了避免打开注册 或者钱包管理时候重复调用
                        {
                            pageNum = 1;
                            presenter.getActiveCRlist(pageNum, pageSize, CRListFragment.this);
                        }
                    }
                });
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();

        if (integer == RxEnum.UPDATAPROPERTY.ordinal()) {
            //子钱包改变  创建或删除
            addDIDPresenter.getAllSubWallets(wallet.getWalletId(), this);
        }
        if (integer == RxEnum.AGREE.ordinal()) {
            //注册did同意了协议
            if (did != null) {
                Bundle bundle = new Bundle();
                bundle.putString("did", did);
                bundle.putSerializable("netList", netList);
                start(CRSignUpForFragment.class, bundle);
                return;
            }
            //注册cr前的判断
            addDIDPresenter.getAllSubWallets(wallet.getWalletId(), this);
            click = true;
        }


    }

    private void setVoterate(CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean, String totalvotes) {
        String voterate = Arith.div(bean.getVotes(), totalvotes, 5).toPlainString();
        voterate = NumberiUtil.numberFormat(Arith.mul(voterate, 100), 5);
        bean.setVoterate(voterate);
    }
}
