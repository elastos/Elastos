package org.elastos.wallet.ela.ui.Assets;


import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.scwang.smartrefresh.layout.SmartRefreshLayout;
import com.scwang.smartrefresh.layout.api.RefreshLayout;
import com.scwang.smartrefresh.layout.constant.RefreshState;
import com.scwang.smartrefresh.layout.listener.OnRefreshListener;

import org.elastos.wallet.R;
import org.elastos.wallet.core.SubWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.listener.RealmTransactionAbs;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.adapter.AssetskAdapter;
import org.elastos.wallet.ela.ui.Assets.bean.AssetsItemEntity;
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
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 资产首页
 */
public class AssetskFragment extends BaseFragment implements AssetsViewData, CommonRvListener1, ISubWalletListener, OnRefreshListener, CommonBalanceViewData {

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
    private List<AssetsItemEntity> assetList;
    private CommonGetBalancePresenter commonGetBalancePresenter;


    @Override
    protected int getLayoutId() {
        initClassicsHeader();
        return R.layout.fragment_assetsk;
    }


    @Override
    protected void initView(View view) {
        onErrorRefreshLayout(srl);
        srl.setOnRefreshListener(this);
        assetsPresenter = new AssetsPresenter();
        commonGetBalancePresenter = new CommonGetBalancePresenter();
        realmUtil = new RealmUtil();
        wallet = realmUtil.queryDefauleWallet();
        setWalletView(wallet);
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleLeft.setImageResource(R.mipmap.aaset_wallet_list);
        ivTitleRight.setImageResource(R.mipmap.asset_wallet_setting);

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
                for (AssetsItemEntity iSubWallet : assetList) {
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
        if (assetskAdapter == null) {
            recyclerview.setAdapter(assetskAdapter = new AssetskAdapter(getContext(), assetList));
            recyclerview.setLayoutManager(new LinearLayoutManager(getContext()));
            recyclerview.setHasFixedSize(true);
            recyclerview.setNestedScrollingEnabled(false);
            recyclerview.setFocusableInTouchMode(false);
            assetskAdapter.setCommonRvListener(this);
        } else {

            assetskAdapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onGetAllSubWallets(List<SubWallet> data) {
        if (assetList == null) {
            assetList = new ArrayList<>();
        }

        assetList.clear();

        // assetsPresenter.getBalance(wallet.getWalletId(), list.get(0).GetChainId(), this);
        for (SubWallet iSubWallet : data) {
            //初始化map
            AssetsItemEntity assetsItemEntity = new AssetsItemEntity();
            assetsItemEntity.setChainId(iSubWallet.GetChainID());
            assetsItemEntity.setBalance(iSubWallet.GetBalance(SubWallet.BalanceType.Total) + "");
            String syncTime = new RealmUtil().querySubWalletSyncTime(wallet.getWalletId(), iSubWallet.GetChainID());
            if (TextUtils.isEmpty(syncTime)) {
                assetsItemEntity.setProgress(0);
                assetsItemEntity.setSyncRime("- -");
            } else {
                assetsItemEntity.setProgress(100);
                assetsItemEntity.setSyncRime(syncTime);
            }
            assetList.add(assetsItemEntity);
            //同步各个字钱包
            //iSubWallet.GetChainId()
            assetsPresenter.registerWalletListener(wallet.getWalletId(), iSubWallet.GetChainID(), this);
        }
        setRecycleView();
    }

    @Override
    public void onBalance(BalanceEntity data) {
        for (AssetsItemEntity assetsItemEntity : assetList) {
            if (assetsItemEntity.getChainId().equals(data.getChainId())) {
                assetsItemEntity.setBalance(data.getBalance());
                post(RxEnum.UPDATAPROGRESS.ordinal(), null, null);
            }

        }
    }

    @Override
    public void onRvItemClick(View view, int position, Object o) {
        Bundle bundle = new Bundle();
        bundle.putString("ChainId", (String) o);
        bundle.putParcelable("wallet", wallet);
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

            if (!wallet.getWalletId().equals(MasterWalletID)) {
                return;
            }
            long lastBlockTime = jsonObject.getLong("lastBlockTime");
            String ChaiID = jsonObject.getString("ChaiID");
            int currentBlockHeight = jsonObject.getInt("currentBlockHeight");
            int estimatedHeight = jsonObject.getInt("estimatedHeight");
            //同步进行中
            int progress = (int) (1.0f * currentBlockHeight / estimatedHeight * 100);
            for (AssetsItemEntity assetsItemEntity : assetList) {
                if (ChaiID.equals(assetsItemEntity.getChainId())) {
                    assetsItemEntity.setProgress(progress);
                    //if (currentBlockHeight >= estimatedHeight) {
                    if (lastBlockTime!=0) {
                        //同步完成
                        String curentTime = DateUtil.time(lastBlockTime);
                        realmUtil.updateWalletSyncTime(wallet.getWalletId(), ChaiID, curentTime, new RealmTransactionAbs() {
                            @Override
                            public void onSuccess() {
                                assetsItemEntity.setSyncRime(curentTime);
                                post(RxEnum.UPDATAPROGRESS.ordinal(), null, null);
                            }
                        });
                    }
                }
            }
            post(RxEnum.UPDATAPROGRESS.ordinal(), null, null);

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
            if (!wallet.getWalletId().equals(MasterWalletID)) {
                return;
            }
            String ChaiID = jsonObject.getString("ChaiID");
            long balance = jsonObject.getLong("Balance");
            for (AssetsItemEntity assetsItemEntity : assetList) {
                if (ChaiID.equals(assetsItemEntity.getChainId())) {
                    assetsItemEntity.setBalance(balance + "");
                    post(RxEnum.UPDATAPROGRESS.ordinal(), null, null);
                    post(RxEnum.BALANCECHANGE.ordinal(), ChaiID, balance);
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
        for (AssetsItemEntity assetsItemEntity : assetList) {
            //初始化map
            commonGetBalancePresenter.getBalance(wallet.getWalletId(), assetsItemEntity.getChainId(), 2, this);
        }

        // assetsPresenter.getAllSubWallets(wallet.getWalletId(), this); Sub wallet callback 3873434088 already exist
        if (refreshLayout.getState() == RefreshState.Refreshing) {
            refreshLayout.finishRefresh();
        }
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
            if (wallet.getWalletId().equals(walletId)) {
                wallet = realmUtil.queryDefauleWallet();
                setWalletView(wallet);
            }
        }
        if (integer == RxEnum.UPDATAPROPERTY.ordinal()) {
            //资产改变
            setWalletView(wallet);
        }
        if (integer == RxEnum.UPDATAPROGRESS.ordinal()) {
            //资产改变
            assetskAdapter.notifyDataSetChanged();
        }

    }
}
