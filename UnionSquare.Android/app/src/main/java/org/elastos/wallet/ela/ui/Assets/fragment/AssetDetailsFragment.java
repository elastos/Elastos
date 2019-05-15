package org.elastos.wallet.ela.ui.Assets.fragment;


import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.scwang.smartrefresh.layout.SmartRefreshLayout;
import com.scwang.smartrefresh.layout.api.RefreshLayout;
import com.scwang.smartrefresh.layout.listener.OnLoadMoreListener;
import com.scwang.smartrefresh.layout.listener.OnRefreshListener;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.adapter.TransferRecordRecAdapetr;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.bean.TransferRecordEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetBalancePresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetTransactionPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonGetTransactionViewData;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 资产详情
 */
public class AssetDetailsFragment extends BaseFragment implements CommonRvListener, CommonGetTransactionViewData, OnRefreshListener, OnLoadMoreListener, CommonBalanceViewData {


    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.srl)
    SmartRefreshLayout srl;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.tv_2)
    TextView tv2;
    @BindView(R.id.iv_2)
    ImageView iv2;
    @BindView(R.id.tv_balance)
    TextView tvBalance;
    @BindView(R.id.tv_synctime)
    TextView tvSynctime;
    @BindView(R.id.tv_record_bg)
    TextView tvRecordBg;
    @BindView(R.id.tv_balanceuse)
    TextView tvBalanceuse;
    private TransferRecordRecAdapetr adapter;
    private List<TransferRecordEntity.TransactionsBean> list;
    private String chainId;
    private Wallet wallet;
    private int startCount = 0;
    private final int pageCount = 20;
    private CommonGetTransactionPresenter presenter;
    private SubWallet subWallet;

    @Override
    protected int getLayoutId() {
        initClassicsFooter();
        initClassicsHeader();
        return R.layout.fragment_asset_details;
    }

    @Override
    protected void setExtraData(Bundle data) {
        chainId = data.getString("ChainId", "ELA");
        wallet = data.getParcelable("wallet");
        subWallet = data.getParcelable("subWallet");
        tvTitle.setText(chainId);
    }

    @Override
    protected void initView(View view) {
        onErrorRefreshLayout(srl);
        if (!chainId.equals(MyWallet.ELA)) {
            tv2.setText(getString(R.string.main_chain_withdraw));
            iv2.setImageResource(R.mipmap.asset_trade_main_withdraw);
        }
        srl.setOnRefreshListener(this);
        srl.setOnLoadMoreListener(this);
        new CommonGetBalancePresenter().getBalance(wallet.getWalletId(), chainId, 2, this);
        presenter = new CommonGetTransactionPresenter();
        //String synctime = new RealmUtil().querySubWalletSyncTime(wallet.getWalletId(), chainId);
        if (subWallet!=null){
            tvSynctime.setText(getString(R.string.lastsynctime) + subWallet.getSyncTime());
        }

        registReceiver();
    }

    //OnBalanceChanged
    @OnClick({R.id.ll_1, R.id.ll_2, R.id.ll_3})
    public void onViewClicked(View view) {
        Bundle bundle = null;
        switch (view.getId()) {
            case R.id.ll_1:
                //转账
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                bundle.putString("ChainId", chainId);
                start(TransferFragment.class, bundle);
                break;
            case R.id.ll_2:
                //充值
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                bundle.putString("ChainId", chainId);
                if (!chainId.equals(MyWallet.ELA)) {
                    tv2.setText(getString(R.string.main_chain_withdraw));
                    iv2.setImageResource(R.mipmap.asset_trade_main_withdraw);
                    start(MainChainWithDrawFragment.class, bundle);
                    break;
                }
                start(SideChainRechargeFragment.class, bundle);

                break;
            case R.id.ll_3:
                //收款
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                bundle.putString("ChainId", chainId);
                start(ReceiptFragment.class, bundle);
                break;
        }
    }


    private void setRecycleView(TransferRecordEntity entity) {
        List<TransferRecordEntity.TransactionsBean> data = entity.getTransactions();
        if (startCount == 0 && (data == null || data.size() == 0)) {
            rv.setVisibility(View.GONE);
            tvRecordBg.setVisibility(View.VISIBLE);
            return;
        } else {
            rv.setVisibility(View.VISIBLE);
            tvRecordBg.setVisibility(View.GONE);
        }

        if (startCount == 0) {
            if (list == null) {
                list = new ArrayList<>();
            } else {
                list.clear();
            }
        } else if (data == null || data.size() == 0) {
            showToastMessage(getString(R.string.loadall));
            return;
        }
        list.addAll(data);
        if (adapter == null) {
            adapter = new TransferRecordRecAdapetr(getContext(), list, chainId);
            rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv.setAdapter(adapter);
            adapter.setCommonRvListener(this);

        } else {
            adapter.notifyDataSetChanged();
        }
        startCount += data.size();
    }

    @Override
    public void onRvItemClick(int position, Object o) {
        Bundle bundle = new Bundle();
        bundle.putString("ChainId", chainId);
        bundle.putParcelable("wallet", wallet);
        bundle.putString("TxHash", ((TransferRecordEntity.TransactionsBean) o).getTxHash());
        start(TransferDetailFragment.class, bundle);
    }


    @Override
    public void onGetAllTransaction(String data) {
        if (srl != null) {
            srl.finishRefresh();
            srl.finishLoadMore();
        }
        TransferRecordEntity transferRecordEntity = JSON.parseObject(data, TransferRecordEntity.class);
        setRecycleView(transferRecordEntity);
    }

    @Override
    public void onRefresh(RefreshLayout refreshLayout) {
        startCount = 0;
        presenter.getAllTransaction(wallet.getWalletId(), chainId, startCount, pageCount, "", this);
    }

    @Override
    public void onLoadMore(RefreshLayout refreshLayout) {
        presenter.getAllTransaction(wallet.getWalletId(), chainId, startCount, startCount + pageCount, "", this);
    }

    @Override
    public void onBalance(BalanceEntity data) {
        if (data != null) {
            tvBalance.setText(NumberiUtil.maxNumberFormat(Arith.div(data.getBalance(), MyWallet.RATE_S), 12) + " ELA");
            presenter.getAllTransaction(wallet.getWalletId(), chainId, startCount, pageCount, "", this);
        }
    }


    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.BALANCECHANGE.ordinal()) {
            SubWallet subWallet = (SubWallet) result.getObj();
            if (subWallet != null && subWallet.getBelongId().equals(wallet.getWalletId()) &&
                    subWallet.getChainId().equals(chainId)) {
                tvBalance.setText(NumberiUtil.maxNumberFormat(Arith.div(subWallet.getBalance(), MyWallet.RATE_S), 12) + chainId);
            }
        }
        if (integer == RxEnum.UPDATAPROGRESS.ordinal()) {
            SubWallet subWallet = (SubWallet) result.getObj();
            if (subWallet != null && subWallet.getBelongId().equals(wallet.getWalletId()) &&
                    subWallet.getChainId().equals(chainId)) {
                tvSynctime.setText(getString(R.string.lastsynctime) + subWallet.getSyncTime());
            }
        }

    }
}
