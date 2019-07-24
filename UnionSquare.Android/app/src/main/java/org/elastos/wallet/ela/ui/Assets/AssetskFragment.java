package org.elastos.wallet.ela.ui.Assets;


import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.scwang.smartrefresh.layout.SmartRefreshLayout;
import com.scwang.smartrefresh.layout.api.RefreshLayout;
import com.scwang.smartrefresh.layout.constant.RefreshState;
import com.scwang.smartrefresh.layout.listener.OnRefreshListener;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.adapter.AssetskAdapter;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.fragment.AddAssetFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.AssetDetailsFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.WalletListFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.WallletManageFragment;
import org.elastos.wallet.ela.ui.Assets.listener.ISubWalletListener;
import org.elastos.wallet.ela.ui.Assets.presenter.AssetsPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetBalancePresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.AssetsViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 资产首页
 */
public class AssetskFragment extends BaseFragment implements AssetsViewData, CommonRvListener1, ISubWalletListener, OnRefreshListener, CommonBalanceViewData, CommmonStringWithMethNameViewData {

    private static final long WAIT_TIME = 2000L;

    @BindView(R.id.iv_title_left)
    ImageView ivTitleLeft;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.iv_add)
    ImageView ivAdd;
    private long TOUCH_TIME = 0;
    @BindView(R.id.recyclerview)
    RecyclerView recyclerview;
    @BindView(R.id.srl)
    SmartRefreshLayout srl;
    private AssetskAdapter assetskAdapter;
    private AssetsPresenter assetsPresenter;
    private RealmUtil realmUtil;
    private Wallet wallet;
    private CommonGetBalancePresenter commonGetBalancePresenter;
    private Map<String, List<org.elastos.wallet.ela.db.table.SubWallet>> listMap;

    @Override
    protected int getLayoutId() {
        initClassicsHeader();
        return R.layout.fragment_assetsk;
    }

    @Override

    public void onSaveInstanceState(Bundle outState) {
        Log.d(getClass().getName(), "onSaveInstanceState");
        realmUtil.updateSubWalletDetial(listMap);
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void initView(View view) {
        onErrorRefreshLayout(srl);
        srl.setOnRefreshListener(this);
        assetsPresenter = new AssetsPresenter();
        commonGetBalancePresenter = new CommonGetBalancePresenter();
        realmUtil = new RealmUtil();
        wallet = realmUtil.queryDefauleWallet();
        tvTitle.setText(wallet.getWalletName());
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleLeft.setImageResource(R.mipmap.aaset_wallet_list);
        ivTitleRight.setImageResource(R.mipmap.asset_wallet_setting);
        List<Wallet> wallets = realmUtil.queryUserAllWallet();
        listMap = new HashMap<>();
        for (Wallet wallet : wallets) {
            assetsPresenter.getAllSubWallets(wallet.getWalletId(), this);
        }
        registReceiver();
    }


    @OnClick({R.id.iv_title_left, R.id.iv_title_right, R.id.iv_add})
    public void onViewClicked(View view) {
        Bundle bundle = null;
        switch (view.getId()) {
            case R.id.iv_title_left:
                //钱包列表
                ((BaseFragment) getParentFragment()).start(new WalletListFragment());
                break;
            case R.id.iv_title_right:
                //钱包管理
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                ((BaseFragment) getParentFragment()).start(WallletManageFragment.class, bundle);
                break;
            case R.id.iv_add:
                bundle = new Bundle();
                bundle.putString("walletId", wallet.getWalletId());
                ArrayList<String> chainIds = new ArrayList<>();
                List<org.elastos.wallet.ela.db.table.SubWallet> assetList = listMap.get(wallet.getWalletId());
                for (org.elastos.wallet.ela.db.table.SubWallet iSubWallet : assetList) {
                    chainIds.add(iSubWallet.getChainId());
                }
                bundle.putStringArrayList("chainIds", chainIds);
                ((BaseFragment) getParentFragment()).start(AddAssetFragment.class, bundle);
                break;

        }
    }

    public static AssetskFragment newInstance() {
        Bundle args = new Bundle();
        AssetskFragment fragment = new AssetskFragment();
        fragment.setArguments(args);
        return fragment;
    }


    private void setWalletView(Wallet wallet) {
        tvTitle.setText(wallet.getWalletName());
        assetsPresenter.getAllSubWallets(wallet.getWalletId(), this);

    }

    private void setRecycleView() {
        List<org.elastos.wallet.ela.db.table.SubWallet> assetList = listMap.get(wallet.getWalletId());
        if (assetList == null || assetList.size() == 0) {
            return;
        }
        if (assetskAdapter == null) {
            recyclerview.setAdapter(assetskAdapter = new AssetskAdapter(getContext(), assetList));
            recyclerview.setLayoutManager(new LinearLayoutManager(getContext()));
            recyclerview.setHasFixedSize(true);
            recyclerview.setNestedScrollingEnabled(false);
            recyclerview.setFocusableInTouchMode(false);
            assetskAdapter.setCommonRvListener(this);
        } else {
            assetskAdapter.setData(assetList);
            assetskAdapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onGetAllSubWallets(List<SubWallet> data, int type) {
      /*  if (type == 1) {
            for (SubWallet newSubWallet : data) {
                if (newSubWallet.getBelongId().equals(wallet.getWalletId())) {
                    assetsPresenter.syncStart(wallet.getWalletId(), newSubWallet.getChainId(), this);
                }
            }
            return;
        }*/
        if (data == null || data.size() == 0) {
            return;
        }
        String currentBelongId = data.get(0).getBelongId();
        List<SubWallet> assetList = listMap.get(currentBelongId);//原来的数据

        for (SubWallet newSubWallet : data) {

            First:
            {
                if (assetList != null && assetList.size() != 0) {
                    for (SubWallet oldSubWallet : assetList) {
                        if (newSubWallet.equals(oldSubWallet)) {
                            //原有的数据保留
                            newSubWallet.setProgress(oldSubWallet.getProgress());
                            newSubWallet.setSyncTime(oldSubWallet.getSyncTime());
                            // assetsPresenter.registerWalletListener(currentBelongId, newSubWallet.getChainId(), this);
                            break First;

                        }

                    }
                    SubWallet subWallet = realmUtil.querySubWallet(newSubWallet.getBelongId(), newSubWallet.getChainId());
                    newSubWallet.setProgress(subWallet.getProgress());
                    newSubWallet.setSyncTime(subWallet.getSyncTime());
                    assetsPresenter.registerWalletListener(currentBelongId, newSubWallet.getChainId(), this);
                } else {
                    SubWallet subWallet = realmUtil.querySubWallet(newSubWallet.getBelongId(), newSubWallet.getChainId());
                    newSubWallet.setProgress(subWallet.getProgress());
                    newSubWallet.setSyncTime(subWallet.getSyncTime());
                    assetsPresenter.registerWalletListener(currentBelongId, newSubWallet.getChainId(), this);
                }
            }

        }

        listMap.put(currentBelongId, data);
        if (wallet.getWalletId().equals(currentBelongId)) {
            setRecycleView();
        }
    }

    @Override
    public void onBalance(BalanceEntity data) {

        List<org.elastos.wallet.ela.db.table.SubWallet> assetList = listMap.get(data.getMasterWalletId());
        for (org.elastos.wallet.ela.db.table.SubWallet assetsItemEntity : assetList) {
            if (assetsItemEntity.getChainId().equals(data.getChainId())) {
                assetsItemEntity.setBalance(data.getBalance());
                if (wallet.getWalletId().equals(data.getMasterWalletId())) {
                    post(RxEnum.BALANCECHANGE.ordinal(), null, assetsItemEntity);
                }
            }

        }
    }

    @Override
    public void onRvItemClick(View view, int position, Object o) {
        Bundle bundle = new Bundle();
        bundle.putString("ChainId", (String) o);
        bundle.putParcelable("wallet", wallet);
        for (SubWallet subWallet : listMap.get(wallet.getWalletId())) {
            if (subWallet.getChainId().equals((String) o)) {
                bundle.putParcelable("subWallet", subWallet);
            }
        }

        ((BaseFragment) getParentFragment()).start(AssetDetailsFragment.class, bundle);
    }

    /***************ISubWalletListener子钱包同步的监听*****************/
    @Override
    public void OnTransactionStatusChanged(JSONObject jsonObject) {
        // Log.e("???", jsonObject.toString());
        //交易状态改变  tx: 19a3dda6e200416ad2c8eaa62047c1211089e594bb0cd70e788c26075fffd9bd, status: Added, confirms: 0
        //交易状态改变  tx: 19a3dda6e200416ad2c8eaa62047c1211089e594bb0cd70e788c26075fffd9bd, status: Added, confirms: 1
    }

    @Override
    public void OnBlockSyncStarted(JSONObject jsonObject) {

        //   Log.e("???", jsonObject.toString());
    }

    @Override
    public synchronized void OnBlockSyncProgress(JSONObject jsonObject) {

        try {
            String MasterWalletID = jsonObject.getString("MasterWalletID");
            long lastBlockTime = jsonObject.getLong("lastBlockTime");
            String ChaiID = jsonObject.getString("ChaiID");
            int currentBlockHeight = jsonObject.getInt("currentBlockHeight");
            int estimatedHeight = jsonObject.getInt("estimatedHeight");
            //同步进行中
            int progress = (int) (1.0f * currentBlockHeight / estimatedHeight * 100);
            List<org.elastos.wallet.ela.db.table.SubWallet> assetList = listMap.get(MasterWalletID);
            for (org.elastos.wallet.ela.db.table.SubWallet subWallet : assetList) {
                if (ChaiID.equals(subWallet.getChainId())) {
                    subWallet.setProgress(progress);
                    if (lastBlockTime != 0) {
                        String curentTime = DateUtil.time(lastBlockTime);
                        subWallet.setSyncTime(curentTime);
                    }
                    subWallet.setProgress(progress);
                    if (wallet.getWalletId().equals(MasterWalletID)) {
                        post(RxEnum.UPDATAPROGRESS.ordinal(), null, subWallet);
                    }
                }
            }


        } catch (JSONException e) {
            e.printStackTrace();
        }

    }

    @Override
    public void OnBlockSyncStopped(JSONObject jsonObject) {
        //同步完成  记录时间并更新
        // Log.e("???", jsonObject.toString());
    }

    @Override
    public synchronized void OnBalanceChanged(JSONObject jsonObject) {

        try {
            String MasterWalletID = jsonObject.getString("MasterWalletID");
            String ChaiID = jsonObject.getString("ChaiID");
            long balance = jsonObject.getLong("Balance");
            List<org.elastos.wallet.ela.db.table.SubWallet> assetList = listMap.get(MasterWalletID);
            for (org.elastos.wallet.ela.db.table.SubWallet assetsItemEntity : assetList) {
                if (ChaiID.equals(assetsItemEntity.getChainId())) {
                    assetsItemEntity.setBalance(balance + "");
                    if (wallet.getWalletId().equals(MasterWalletID)) {
                        post(RxEnum.BALANCECHANGE.ordinal(), null, assetsItemEntity);
                    }

                }
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }

    }

    @Override
    public void OnTxPublished(JSONObject jsonObject) {
        // Log.e("???", jsonObject.toString());
        //交易成功   OnTxPublished => 19a3dda6e200416ad2c8eaa62047c1211089e594bb0cd70e788c26075fffd9bd, result: {"Code":0,"Reason":"has tx"}
    }

    @Override
    public void OnTxDeleted(JSONObject jsonObject) {
        //  Log.e("???", jsonObject.toString());
    }

    @Override
    public void onRefresh(RefreshLayout refreshLayout) {
        List<org.elastos.wallet.ela.db.table.SubWallet> assetList = listMap.get(wallet.getWalletId());
        for (org.elastos.wallet.ela.db.table.SubWallet assetsItemEntity : assetList) {
            //初始化map
            commonGetBalancePresenter.getBalance(wallet.getWalletId(), assetsItemEntity.getChainId(), 2, this);
        }

        // assetsPresenter.getAllSubWallets(wallet.getWalletId(), this); Sub wallet callback 3873434088 already exist
        if (refreshLayout.getState() == RefreshState.Refreshing) {
            refreshLayout.finishRefresh();
        }
        assetsPresenter.getAllSubWallets(wallet.getWalletId(), 1, this);

    }

    /**
     * 处理回退事件
     *
     * @return
     */
    @Override
    public boolean onBackPressedSupport() {
        if (System.currentTimeMillis() - TOUCH_TIME < WAIT_TIME) {
            _mActivity.finish();
        } else {
            TOUCH_TIME = System.currentTimeMillis();
            Toast.makeText(_mActivity, getString(R.string.press_exit_again), Toast.LENGTH_SHORT).show();
        }
        return true;
    }

    /**
     * eventbus监听
     *
     * @param result
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.WALLETUPDATE.ordinal() || integer == RxEnum.ONE.ordinal()) {
            //切换钱包 或者创建钱包
            Wallet temp = (Wallet) result.getObj();
            if (!wallet.getWalletId().equals(temp.getWalletId())) {
                wallet = temp;
                setWalletView(wallet);
            }

        }
        if (integer == RxEnum.UPDATA_WALLET_NAME.ordinal()) {
            String walletId = (String) result.getObj();
            if (wallet.getWalletId().equals(walletId)) {
                wallet.setWalletName(result.getName());
                tvTitle.setText(result.getName());
            }
        }
        if (integer == RxEnum.DELETE.ordinal()) {
            //删除
            String walletId = (String) result.getObj();
            listMap.remove(walletId);
            if (wallet.getWalletId().equals(walletId)) {
                wallet = realmUtil.queryDefauleWallet();
                setWalletView(wallet);
            }
        }
        if (integer == RxEnum.UPDATAPROPERTY.ordinal()) {
            //子钱包改变  创建或删除
            setWalletView(wallet);
        }
        if (integer == RxEnum.UPDATAPROGRESS.ordinal()) {
            //progress改变
            setRecycleView();
        }
        if (integer == RxEnum.BALANCECHANGE.ordinal()) {
            //资产改变
            setRecycleView();
        }

    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        //存储所有同步状态
        realmUtil.updateSubWalletDetial(listMap);
    }

    @Override
    public void onGetCommonData(String methodname, String data) {

    }
}
