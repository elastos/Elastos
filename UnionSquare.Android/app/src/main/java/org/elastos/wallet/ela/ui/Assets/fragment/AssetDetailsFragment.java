package org.elastos.wallet.ela.ui.Assets.fragment;


import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
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
import org.elastos.wallet.ela.ui.Assets.activity.TransferActivity;
import org.elastos.wallet.ela.ui.Assets.adapter.TransferRecordRecAdapetr;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.bean.TransferRecordEntity;
import org.elastos.wallet.ela.ui.Assets.fragment.transfer.SignFragment;
import org.elastos.wallet.ela.ui.Assets.presenter.AssetDetailPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.AssetsPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetBalancePresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetTransactionPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonGetTransactionViewData;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.find.presenter.VoteFirstPresenter;
import org.elastos.wallet.ela.ui.find.viewdata.RegisteredProducerInfoViewData;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 资产详情
 */
public class AssetDetailsFragment extends BaseFragment implements CommonRvListener, CommonGetTransactionViewData, OnRefreshListener, OnLoadMoreListener, CommonBalanceViewData, RegisteredProducerInfoViewData, RadioGroup.OnCheckedChangeListener, CommmonStringWithMethNameViewData {


    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.srl)
    SmartRefreshLayout srl;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.rv1)
    RecyclerView rv1;
    @BindView(R.id.tv_chain)
    TextView tvChain;
    @BindView(R.id.tv_balance)
    TextView tvBalance;
    @BindView(R.id.tv_synctime)
    TextView tvSynctime;
    @BindView(R.id.tv_record_bg)
    TextView tvRecordBg;
    @BindView(R.id.tv_earn_bg)
    TextView tvEarnBg;
    @BindView(R.id.tv_balanceuse)
    TextView tvBalanceuse;
    @BindView(R.id.rg_record)
    RadioGroup rgRecord;
    @BindView(R.id.tv_address)
    TextView tvAddress;
    @BindView(R.id.tv_towhole)
    TextView tvTowhole;
    @BindView(R.id.tv_copyaddress)
    TextView tvCopyaddress;
    @BindView(R.id.ll_earn)
    LinearLayout llEarn;
    @BindView(R.id.rb_earn_recorder)
    RadioButton rbEarnRecorder;
    @BindView(R.id.view_line)
    View viewLine;
    private TransferRecordRecAdapetr adapter;
    private TransferRecordRecAdapetr adapter1;
    private List<TransferRecordEntity.TransactionsBean> list;
    private List<TransferRecordEntity.TransactionsBean> list1;
    private String chainId;
    private Wallet wallet;
    private int startCount = 0;
    private int startCount1 = 0;
    private final int pageCount = 20;
    private CommonGetTransactionPresenter presenter;
    private SubWallet subWallet;
    private AssetDetailPresenter assetDetailPresenter;


    @Override
    protected int getLayoutId() {
        initClassicsFooter();
        initClassicsHeader();
        return R.layout.fragment_asset_details;
    }

    @Override
    protected void setExtraData(Bundle data) {

        wallet = data.getParcelable("wallet");
        subWallet = data.getParcelable("subWallet");
        chainId = subWallet.getChainId();
        tvTitle.setText(chainId);


    }

    @Override
    protected void initView(View view) {
        assetDetailPresenter = new AssetDetailPresenter();
        if (chainId.equals(MyWallet.ELA)) {
            tvChain.setText(getString(R.string.side_chain_top_up));
            viewLine.setVisibility(View.VISIBLE);
            tvTowhole.setVisibility(View.VISIBLE);
            new VoteFirstPresenter().getRegisteredProducerInfo(wallet.getWalletId(), chainId, this);
            if ("true".equals(subWallet.getFiled2())) {
                assetDetailPresenter.getAllUTXOs(wallet.getWalletId(), chainId, 0, 1, "", this);
            }
        }
        presenter = new CommonGetTransactionPresenter();
        presenter.getAllTransaction(wallet.getWalletId(), chainId, startCount, pageCount, "", this);
        srl.setOnRefreshListener(this);
        srl.setOnLoadMoreListener(this);
        new CommonGetBalancePresenter().getBalance(wallet.getWalletId(), chainId, 2, this);
        //String synctime = new RealmUtil().querySubWalletSyncTime(wallet.getWalletId(), chainId);
        if (subWallet != null) {
            tvSynctime.setText(getString(R.string.lastsynctime) + DateUtil.time(subWallet.getSyncTime(), getContext()));
        }

        registReceiver();
        rgRecord.setOnCheckedChangeListener(this);
    }

    //OnBalanceChanged
    @OnClick({R.id.ll_transfer, R.id.tv_chain, R.id.ll_recipe, R.id.tv_towhole, R.id.tv_copyaddress})
    public void onViewClicked(View view) {
        Bundle bundle = null;
        switch (view.getId()) {
            case R.id.ll_transfer:
                //转账
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                bundle.putString("ChainID", chainId);
                start(TransferFragment.class, bundle);
                break;
            case R.id.tv_chain:
                //充值
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                bundle.putString("ChainId", chainId);
                if (!chainId.equals(MyWallet.ELA)) {
                    start(MainChainWithDrawFragment.class, bundle);
                    break;
                }
                start(SideChainRechargeFragment.class, bundle);

                break;
            case R.id.ll_recipe:
                //收款
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                bundle.putString("ChainId", chainId);
                start(ReceiptFragment.class, bundle);
                break;
            case R.id.tv_towhole:
                //零钱换整
                assetDetailPresenter.createCombineUTXOTransaction(wallet.getWalletId(), chainId, "", false, this);
                break;
            case R.id.tv_copyaddress:
                ClipboardUtil.copyClipboar(getBaseActivity(), tvAddress.getText().toString());
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

    private void setRecycleView1(TransferRecordEntity entity) {
        List<TransferRecordEntity.TransactionsBean> data = entity.getTransactions();
        if (startCount1 == 0 && (data == null || data.size() == 0)) {
            rv1.setVisibility(View.GONE);
            tvEarnBg.setVisibility(View.VISIBLE);
            return;
        } else {
            rv1.setVisibility(View.VISIBLE);
            tvEarnBg.setVisibility(View.GONE);
        }

        if (startCount1 == 0) {
            if (list1 == null) {
                list1 = new ArrayList<>();
            } else {
                list1.clear();
            }
        } else if (data == null || data.size() == 0) {
            showToastMessage(getString(R.string.loadall));
            return;
        }
        list1.addAll(data);
        if (adapter1 == null) {
            adapter1 = new TransferRecordRecAdapetr(getContext(), list1, chainId);
            rv1.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv1.setAdapter(adapter1);
            adapter1.setCommonRvListener(this);

        } else {
            adapter1.notifyDataSetChanged();
        }
        startCount1 += data.size();
    }

    @Override
    public void onRvItemClick(int position, Object o) {
        Bundle bundle = new Bundle();
        bundle.putString("ChainId", chainId);
        bundle.putParcelable("wallet", wallet);
        bundle.putString("TxHash", ((TransferRecordEntity.TransactionsBean) o).getTxHash());
        if (rbEarnRecorder.isChecked()) {
            bundle.putInt("recordType", 1);//收益记录
        } else {
            bundle.putInt("recordType", 0);//转账记录
        }

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
        onErrorRefreshLayout(srl);
        if (rbEarnRecorder.isChecked()) {
            startCount1 = 0;
            assetDetailPresenter.getAllCoinBaseTransaction(wallet.getWalletId(), chainId, startCount1, pageCount, "", this);
        } else {
            startCount = 0;
            presenter.getAllTransaction(wallet.getWalletId(), chainId, startCount, pageCount, "", this);
        }
    }

    @Override
    public void onLoadMore(RefreshLayout refreshLayout) {
        onErrorRefreshLayout(srl);
        if (rbEarnRecorder.isChecked()) {
            assetDetailPresenter.getAllCoinBaseTransaction(wallet.getWalletId(), chainId, startCount1, pageCount, "", this);
        } else {
            presenter.getAllTransaction(wallet.getWalletId(), chainId, startCount, startCount + pageCount, "", this);
        }
        new AssetsPresenter().syncStart(wallet.getWalletId(), chainId, this);
    }

    @Override
    public void onBalance(BalanceEntity data) {
        if (data != null) {
            tvBalance.setText(NumberiUtil.maxNumberFormat(Arith.div(data.getBalance(), MyWallet.RATE_S), 12) + " ELA");
        }
    }


    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.BALANCECHANGE.ordinal()) {
            SubWallet subWallet = (SubWallet) result.getObj();
            if (subWallet != null && subWallet.getBelongId().equals(wallet.getWalletId()) &&
                    subWallet.getChainId().equals(chainId)) {
                tvBalance.setText(NumberiUtil.maxNumberFormat(Arith.div(subWallet.getBalance(), MyWallet.RATE_S), 12) + " ELA");
            }
        }
        if (integer == RxEnum.UPDATAPROGRESS.ordinal()) {
            SubWallet subWallet = (SubWallet) result.getObj();
            if (subWallet != null && subWallet.getBelongId().equals(wallet.getWalletId()) &&
                    subWallet.getChainId().equals(chainId)) {
                tvSynctime.setText(getString(R.string.lastsynctime) + DateUtil.time(subWallet.getSyncTime(), getContext()));
            }
        }
        if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                }
            });

        }
        if (integer == RxEnum.TOSIGN.ordinal()) {
            //生成待签名交易
            String attributes = (String) result.getObj();
            Bundle bundle = new Bundle();
            bundle.putString("attributes", attributes);
            bundle.putParcelable("wallet", wallet);
            bundle.putInt("transType",2);
            start(SignFragment.class, bundle);

        }
        if (integer == RxEnum.SIGNSUCCESS.ordinal()) {
            //签名成功
            String attributes = (String) result.getObj();
            Bundle bundle = new Bundle();
            bundle.putString("attributes", attributes);
            bundle.putParcelable("wallet", wallet);
            bundle.putInt("transType",2);
            bundle.putBoolean("signStatus", true);
            start(SignFragment.class, bundle);

        }

    }

    @Override
    public void onGetRegisteredProducerInfo(String data) {
        JSONObject jsonObject = JSON.parseObject(data);
        String status = jsonObject.getString("Status");
        if (!TextUtils.isEmpty(status)) {
            switch (status) {
                case "Unregistered":
                    rbEarnRecorder.setVisibility(View.GONE);
                    break;
                case "ReturnDeposit"://取回押金
                case "Canceled":
                case "Registered":
                    rbEarnRecorder.setVisibility(View.VISIBLE);
                    break;

            }

        }
    }

    boolean firstCheck = true;

    @Override
    public void onCheckedChanged(RadioGroup group, int checkedId) {
        switch (checkedId) {
            case R.id.rb_earn_recorder:
                if (firstCheck) {
                    assetDetailPresenter.getOwnerAddress(wallet.getWalletId(), chainId, this);
                    assetDetailPresenter.getAllCoinBaseTransaction(wallet.getWalletId(), chainId, startCount1, pageCount, "", this);
                    firstCheck = false;
                }
                llEarn.setVisibility(View.VISIBLE);
                rv.setVisibility(View.GONE);
                break;
            case R.id.rb_trans_recorder:
                llEarn.setVisibility(View.GONE);
                rv.setVisibility(View.VISIBLE);
                break;
        }
    }


    @Override
    public void onGetCommonData(String methodname, String data) {
        switch (methodname) {
            case "getOwnerAddress":
                tvAddress.setText(data);
                break;
            case "getAllUTXOs":
                JSONObject jsonObject = JSON.parseObject(data);
                if (jsonObject.containsKey("MaxCount")) {
                    int maxCount = jsonObject.getInteger("MaxCount");
                    if (maxCount > 500) {
                        exchange(maxCount);
                    }
                }
                break;
            case "createCombineUTXOTransaction":
                //零钱换整
                Intent intent = new Intent(getActivity(), TransferActivity.class);

                intent.putExtra("wallet", wallet);
                intent.putExtra("attributes", data);
                intent.putExtra("chainId", chainId);
                intent.putExtra("type", Constant.TOWHOL);
                intent.putExtra("transType",2);
                startActivity(intent);
                break;
            case "getAllCoinBaseTransaction":
                if (srl != null) {
                    srl.finishRefresh();
                    srl.finishLoadMore();
                }
                TransferRecordEntity transferRecordEntity = JSON.parseObject(data, TransferRecordEntity.class);
                setRecycleView1(transferRecordEntity);
                break;
        }
    }

    private void exchange(int maxCount) {
        String tip = String.format(getString(R.string.whetherexchange), maxCount + "");

        new DialogUtil().showWarmPrompt1(getBaseActivity(), tip, new WarmPromptListener() {
            @Override
            public void affireBtnClick(View view) {
                assetDetailPresenter.createCombineUTXOTransaction(wallet.getWalletId(), chainId, "", false, AssetDetailsFragment.this);

            }
        });
    }
}
