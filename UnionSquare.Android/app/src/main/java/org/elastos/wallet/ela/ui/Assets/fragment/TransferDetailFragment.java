package org.elastos.wallet.ela.ui.Assets.fragment;

import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.adapter.TransferDetailRecAdapetr;
import org.elastos.wallet.ela.ui.Assets.bean.Payload;
import org.elastos.wallet.ela.ui.Assets.bean.RecorderAddressEntity;
import org.elastos.wallet.ela.ui.Assets.bean.TransferRecordDetailEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetTransactionPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonGetTransactionViewData;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.NumberiUtil;
import com.scwang.smartrefresh.layout.SmartRefreshLayout;
import com.scwang.smartrefresh.layout.api.RefreshLayout;
import com.scwang.smartrefresh.layout.listener.OnRefreshListener;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class TransferDetailFragment extends BaseFragment implements CommonRvListener, CommonGetTransactionViewData, OnRefreshListener {
    @BindView(R.id.tv_title)
    TextView tvTitle;

    Unbinder unbinder;
    @BindView(R.id.tv_transfernum)
    TextView tvTransfernum;
    @BindView(R.id.tv_transferamount)
    TextView tvTransferamount;
    @BindView(R.id.tv_receiptamount)
    TextView tvReceiptamount;
    @BindView(R.id.tv_sendamount)
    TextView tvSendamount;
    @BindView(R.id.tv_charge)
    TextView tvCharge;
    @BindView(R.id.tv_address_in1)
    TextView tvAddressIn1;
    @BindView(R.id.tv_amount_in1)
    TextView tvAmountIn1;
    @BindView(R.id.ll_in1)
    LinearLayout llIn1;
    @BindView(R.id.tv_address_in2)
    TextView tvAddressIn2;
    @BindView(R.id.tv_amount_in2)
    TextView tvAmountIn2;
    @BindView(R.id.ll_in2)
    LinearLayout llIn2;
    @BindView(R.id.iv_show_in)
    ImageView ivShowIn;
    @BindView(R.id.rv_in)
    RecyclerView rvIn;
    @BindView(R.id.iv_hide_in)
    ImageView ivHideIn;
    @BindView(R.id.ll_show_in)
    LinearLayout llShowIn;
    @BindView(R.id.ll_in)
    LinearLayout llIn;
    @BindView(R.id.tv_address_out1)
    TextView tvAddressOut1;
    @BindView(R.id.tv_amount_out1)
    TextView tvAmountOut1;
    @BindView(R.id.ll_out1)
    LinearLayout llOut1;
    @BindView(R.id.tv_address_out2)
    TextView tvAddressOut2;
    @BindView(R.id.tv_amount_out2)
    TextView tvAmountOut2;
    @BindView(R.id.ll_out2)
    LinearLayout llOut2;
    @BindView(R.id.iv_show_out)
    ImageView ivShowOut;
    @BindView(R.id.rv_out)
    RecyclerView rvOut;
    @BindView(R.id.iv_hide_out)
    ImageView ivHideOut;
    @BindView(R.id.ll_show_out)
    LinearLayout llShowOut;
    @BindView(R.id.ll_out)
    LinearLayout llOut;
    @BindView(R.id.tv_suretime)
    TextView tvSuretime;
    @BindView(R.id.tv_suretimes)
    TextView tvSuretimes;
    @BindView(R.id.tv_remark)
    TextView tvRemark;
    Unbinder unbinder1;
    @BindView(R.id.ll_charge)
    LinearLayout llCharge;
    Unbinder unbinder2;
    @BindView(R.id.tv_type)
    TextView tvType;
    @BindView(R.id.srl)
    SmartRefreshLayout srl;
    @BindView(R.id.tv_ticketnum)
    TextView tvTicketnum;
    Unbinder unbinder3;
    @BindView(R.id.ll_ticketnum)
    LinearLayout llTicketnum;
    @BindView(R.id.tv_address_target1)
    TextView tvAddressTarget1;
    @BindView(R.id.tv_amount_target1)
    TextView tvAmountTarget1;
    @BindView(R.id.ll_target)
    LinearLayout llTarget;
    private String type;//00 转入未打包 01转入进行中 02转入已完成

    private TransferDetailRecAdapetr adapterIn;
    private TransferDetailRecAdapetr adapterOut;
    private String txHash;
    private CommonGetTransactionPresenter presenter;
    private String chainId;
    private Wallet wallet;
    private List<RecorderAddressEntity> inputList;
    private List<RecorderAddressEntity> outputList;

    @Override
    protected int getLayoutId() {
        initClassicsHeader();
        return R.layout.fragment_transfer_detail;
    }

    @Override
    protected void setExtraData(Bundle data) {
        txHash = data.getString("TxHash");
        chainId = data.getString("ChainId", "ELA");
        wallet = data.getParcelable("wallet");
        presenter = new CommonGetTransactionPresenter();
        presenter.getAllTransaction(wallet.getWalletId(), chainId, 0, 20, txHash, this);
        tvTransfernum.setText(txHash);
        srl.setOnRefreshListener(this);

    }

    @Override
    protected void initView(View view) {
        onErrorRefreshLayout(srl);
        tvTitle.setText(getString(R.string.exchangedetail));
        // String direction = transactionsBean.getDirection();//direction有3种, Moved ,Received,Sent

    }

    private void setRecycleViewIn() {

        if (adapterIn == null) {
            adapterIn = new TransferDetailRecAdapetr(getContext(), inputList);
            rvIn.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rvIn.setHasFixedSize(true);
            rvIn.setNestedScrollingEnabled(false);
            rvIn.setFocusableInTouchMode(false);
            rvIn.setAdapter(adapterIn);
            adapterIn.setCommonRvListener(this);

        } else {
            adapterIn.notifyDataSetChanged();
        }

    }

    private void setRecycleViewOut() {
        if (adapterOut == null) {
            adapterOut = new TransferDetailRecAdapetr(getContext(), outputList);
            rvOut.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rvOut.setHasFixedSize(true);
            rvOut.setNestedScrollingEnabled(false);
            rvOut.setFocusableInTouchMode(false);
            rvOut.setAdapter(adapterOut);
            adapterOut.setCommonRvListener(this);

        } else {
            adapterOut.notifyDataSetChanged();
        }

    }

    @OnClick({R.id.iv_show_in, R.id.iv_show_out, R.id.iv_hide_in, R.id.iv_hide_out, R.id.tv_transfernum})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_show_in:
                llShowIn.setVisibility(View.VISIBLE);
                ivShowIn.setVisibility(View.GONE);
                break;
            case R.id.iv_show_out:
                llShowOut.setVisibility(View.VISIBLE);
                ivShowOut.setVisibility(View.GONE);
                break;
            case R.id.iv_hide_in:
                llShowIn.setVisibility(View.GONE);
                ivShowIn.setVisibility(View.VISIBLE);
                break;
            case R.id.iv_hide_out:
                llShowOut.setVisibility(View.GONE);
                ivShowOut.setVisibility(View.VISIBLE);
                break;
            case R.id.tv_transfernum:
                //调外链
                if ("ELA".equals(chainId)) {
                    try {
                        startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(MyWallet.Url + txHash)));
                    } catch (ActivityNotFoundException a) {
                        showToastMessage("非法地址,请联系活动发布商");
                    }
                }
                break;

        }
    }

    @Override
    public void onRvItemClick(int position, Object o) {

    }

    @Override
    public void onGetAllTransaction(String data) {
        srl.finishRefresh();
        TransferRecordDetailEntity transferRecordDetailEntity = JSON.parseObject(data, TransferRecordDetailEntity.class);
        if (transferRecordDetailEntity == null) {
            showToast(getString(R.string.nodata));
            return;
        }
        List<TransferRecordDetailEntity.TransactionsBean> transactions = transferRecordDetailEntity.getTransactions();
        if (transactions == null || transactions.size() == 0) {
            showToast(getString(R.string.nodata));
            return;
        }
        TransferRecordDetailEntity.TransactionsBean transactionsBean = transactions.get(0);
        //tvTransferamount.setText(NumberiUtil.maxNumberFormat(((double) transactionsBean.getAmount() / MyWallet.RATE) + "", 12) + " " + chainId);
        tvTransferamount.setText(NumberiUtil.maxNumberFormat(Arith.div(transactionsBean.getAmount() + "", MyWallet.RATE_S), 12) + " ELA");
        if (transactionsBean.getFee() == 0) {
            llCharge.setVisibility(View.GONE);
        } else {
            // tvCharge.setText(NumberiUtil.maxNumberFormat(((double) transactionsBean.getFee() / MyWallet.RATE) + "", 12) + " " + chainId);
            tvCharge.setText(NumberiUtil.maxNumberFormat(Arith.div(transactionsBean.getFee() + "", MyWallet.RATE_S), 12) + " ELA");
        }

        if (transactionsBean.getStatus().equals("Pending")) {
            tvSuretime.setText("- -");
        } else {
            tvSuretime.setText(DateUtil.time(transactionsBean.getTimestamp()));
        }
        tvSuretimes.setText(transactionsBean.getConfirmStatus());
        tvRemark.setText(transactionsBean.getRemark());
        //提现或者充值的特别情况
        if (transactionsBean.getType() == 8) {
            llTarget.setVisibility(View.VISIBLE);
            Payload payload=    JSON.parseObject(transactionsBean.getPayload(), Payload.class);
            if (payload!=null&&payload.getCrossChainAddress()!=null&&payload.getCrossChainAddress().size()!=0){
               tvAddressTarget1.setText(payload.getCrossChainAddress().get(0));
            }
            if (payload!=null&&payload.getCrossChainAmount()!=null&&payload.getCrossChainAmount().size()!=0){
                tvAmountTarget1.setText(NumberiUtil.maxNumberFormat(Arith.div(payload.getCrossChainAmount().get(0), MyWallet.RATE_S), 12) + " ELA");
            }
        }
        int transferType = getContext().getResources().getIdentifier("transfertype" + transactionsBean.getType(), "string",
                getContext().getPackageName());
        try {
            tvType.setText(getString(transferType));
        } catch (Exception e) {
            tvType.setText(getString(R.string.transfertype13));
        }
        List<TransferRecordDetailEntity.TransactionsBean.OutputPayloadBean> outputPayloadBeans = transactionsBean.getOutputPayload();
        if (outputPayloadBeans != null && outputPayloadBeans.size() > 0) {
            //特殊类型 投票交易
            llTicketnum.setVisibility(View.VISIBLE);
            // tvTicketnum.setText(NumberiUtil.maxNumberFormat((outputPayloadBeans.get(0).getAmount() / MyWallet.RATE) + "", 12));
            tvTicketnum.setText(NumberiUtil.maxNumberFormat(Arith.div(outputPayloadBeans.get(0).getAmount() + "", MyWallet.RATE_S), 12));
            tvType.setText(R.string.votetrade);
        }
        String output = transactionsBean.getOutputs();

        if (outputList == null) {
            outputList = new ArrayList<>();
        } else {
            outputList.clear();
        }
        outputList.addAll(initAddressData(output));
        initOutAddressView();

        //input处理
        String input = transactionsBean.getInputs();

        if (inputList == null) {
            inputList = new ArrayList<>();
        } else {
            inputList.clear();
        }
        inputList.addAll(initAddressData(input));
        initInAddressView();
    }

    private void initOutAddressView() {
        int outsize = outputList.size();
        if (outsize == 0) {
            llOut.setVisibility(View.GONE);
        } else if (outsize == 1) {
            llOut2.setVisibility(View.GONE);
            ivShowOut.setVisibility(View.GONE);
            llShowOut.setVisibility(View.GONE);
            tvAddressOut1.setText(outputList.get(0).getAddress());
            tvAmountOut1.setText(outputList.get(0).getAmount());

        } else if (outsize == 2) {
            ivShowOut.setVisibility(View.GONE);
            llShowOut.setVisibility(View.GONE);
            tvAddressOut1.setText(outputList.get(0).getAddress());
            tvAmountOut1.setText(outputList.get(0).getAmount());
            tvAddressOut2.setText(outputList.get(1).getAddress());
            tvAmountOut2.setText(outputList.get(1).getAmount());
        } else {
            tvAddressOut1.setText(outputList.get(0).getAddress());
            tvAmountOut1.setText(outputList.get(0).getAmount());
            tvAddressOut2.setText(outputList.get(1).getAddress());
            tvAmountOut2.setText(outputList.get(1).getAmount());
            setRecycleViewOut();
        }
    }

    private void initInAddressView() {
        int insize = inputList.size();
        if (insize == 0) {
            llIn.setVisibility(View.GONE);
        } else if (insize == 1) {
            llIn2.setVisibility(View.GONE);
            ivShowIn.setVisibility(View.GONE);
            llShowIn.setVisibility(View.GONE);
            tvAddressIn1.setText(inputList.get(0).getAddress());
            tvAmountIn1.setText(inputList.get(0).getAmount());

        } else if (insize == 2) {
            ivShowIn.setVisibility(View.GONE);
            llShowIn.setVisibility(View.GONE);
            tvAddressIn1.setText(inputList.get(0).getAddress());
            tvAmountIn1.setText(inputList.get(0).getAmount());
            tvAddressIn2.setText(inputList.get(1).getAddress());
            tvAmountIn2.setText(inputList.get(1).getAmount());
        } else {
            tvAddressIn1.setText(inputList.get(0).getAddress());
            tvAmountIn1.setText(inputList.get(0).getAmount());
            tvAddressIn2.setText(inputList.get(1).getAddress());
            tvAmountIn2.setText(inputList.get(1).getAmount());
            setRecycleViewIn();
        }
    }

    private List<RecorderAddressEntity> initAddressData(String putJson) {
        JSONObject jsonObject = JSON.parseObject(putJson);
        List<RecorderAddressEntity> list = new ArrayList<>();

        for (String key : jsonObject.keySet()) {
            if (key == null) {
                break;
            }
            // String amount = NumberiUtil.maxNumberFormat(((double) jsonObject.getLong(key) / MyWallet.RATE) + "", 12);
            String amount = NumberiUtil.maxNumberFormat(Arith.div(jsonObject.getLong(key) + "", MyWallet.RATE_S), 12);
            list.add(new RecorderAddressEntity(key, amount + " ELA"));

        }
        return list;
    }


    @Override
    public void onRefresh(RefreshLayout refreshLayout) {
        presenter.getAllTransaction(wallet.getWalletId(), chainId, 0, 20, txHash, this);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        // TODO: inflate a fragment view
        View rootView = super.onCreateView(inflater, container, savedInstanceState);
        unbinder3 = ButterKnife.bind(this, rootView);
        return rootView;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder3.unbind();
    }
}
