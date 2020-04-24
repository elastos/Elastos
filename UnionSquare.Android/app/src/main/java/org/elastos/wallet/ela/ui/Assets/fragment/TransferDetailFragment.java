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

package org.elastos.wallet.ela.ui.Assets.fragment;

import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.scwang.smartrefresh.layout.SmartRefreshLayout;
import com.scwang.smartrefresh.layout.api.RefreshLayout;
import com.scwang.smartrefresh.layout.listener.OnRefreshListener;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.adapter.TransferDetailRecAdapetr;
import org.elastos.wallet.ela.ui.Assets.bean.CoinBaseTransferRecordEntity;
import org.elastos.wallet.ela.ui.Assets.bean.RecorderAddressEntity;
import org.elastos.wallet.ela.ui.Assets.bean.TransferRecordDetailEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.AssetDetailPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetTransactionPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonGetTransactionViewData;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.NumberiUtil;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class TransferDetailFragment extends BaseFragment implements CommonRvListener, CommonGetTransactionViewData, OnRefreshListener, CommmonStringWithMethNameViewData {
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
    @BindView(R.id.ll_charge)
    LinearLayout llCharge;
    @BindView(R.id.tv_type)
    TextView tvType;
    @BindView(R.id.srl)
    SmartRefreshLayout srl;
    @BindView(R.id.tv_ticketnum)
    TextView tvTicketnum;
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
    private int recordType;
    AssetDetailPresenter assetDetailPresenter;

    @Override
    protected int getLayoutId() {
        initClassicsHeader();
        return R.layout.fragment_transfer_detail;
    }

    @Override
    protected void setExtraData(Bundle data) {
        txHash = data.getString("TxHash");
        recordType = data.getInt("recordType", 0);
        chainId = data.getString("ChainId", "ELA");
        wallet = data.getParcelable("wallet");

        if (recordType == 1) {
            //收益记录详情
            assetDetailPresenter = new AssetDetailPresenter();
            assetDetailPresenter.getAllCoinBaseTransaction(wallet.getWalletId(), chainId, 0, 20, txHash, this);

        } else {
            //交易记录
            presenter = new CommonGetTransactionPresenter();
            presenter.getAllTransaction(wallet.getWalletId(), chainId, 0, 20, txHash, this);
        }
        tvTransfernum.setText(txHash);
        srl.setOnRefreshListener(this);

    }

    @Override
    protected void initView(View view) {
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
            tvSuretime.setText(DateUtil.time(transactionsBean.getTimestamp(), getContext()));
        }
        tvSuretimes.setText(transactionsBean.getConfirmStatus());
        tvRemark.setText(transactionsBean.getMemo());
        //提现或者充值的特别情况
        if (transactionsBean.getType() == 8) {
            llTarget.setVisibility(View.VISIBLE);
            if (transactionsBean.getPayload() != null && transactionsBean.getPayload().size() != 0) {
                TransferRecordDetailEntity.TransactionsBean.PayloadBean payloadBean = transactionsBean.getPayload().get(0);
                tvAddressTarget1.setText(payloadBean.getCrossChainAddress());
                tvAmountTarget1.setText(NumberiUtil.maxNumberFormat(Arith.div(payloadBean.getCrossChainAmount(), MyWallet.RATE_S), 12) + " ELA");
            }
        }
        int transferType;
        if (chainId.equals(MyWallet.IDChain)) {
            transferType = getContext().getResources().getIdentifier("sidetransfertype" + transactionsBean.getType(), "string",
                    getContext().getPackageName());
        } else {
            transferType = getContext().getResources().getIdentifier("transfertype" + transactionsBean.getType(), "string",
                    getContext().getPackageName());
        }


        List<TransferRecordDetailEntity.TransactionsBean.OutputPayloadBean> outputPayloadBeans = transactionsBean.getOutputPayload();
        if (outputPayloadBeans != null && outputPayloadBeans.size() > 0) {
            //特殊类型 投票交易
            // llTicketnum.setVisibility(View.VISIBLE);
            //  tvTicketnum.setText(NumberiUtil.maxNumberFormat(Arith.div(outputPayloadBeans.get(0).getAmount() + "", MyWallet.RATE_S), 12));
            transferType = getContext().getResources().getIdentifier("transfertype" + 1001, "string",
                    getContext().getPackageName());
        }
        try {
            tvType.setText(getString(transferType));
        } catch (Exception e) {
            tvType.setText(getString(R.string.transfertype13));
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
        if (jsonObject == null) {
            return list;
        }
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
        onErrorRefreshLayout(srl);
        if (recordType == 0) {
            presenter.getAllTransaction(wallet.getWalletId(), chainId, 0, 20, txHash, this);
        } else {
            assetDetailPresenter.getAllCoinBaseTransaction(wallet.getWalletId(), chainId, 0, 20, txHash, this);
        }
    }

    @Override
    public void onGetCommonData(String methodname, String data) {
        //getAllCoinBaseTransaction 获得收益记录
        srl.finishRefresh();
        CoinBaseTransferRecordEntity transferRecordDetailEntity = JSON.parseObject(data, CoinBaseTransferRecordEntity.class);
        if (transferRecordDetailEntity == null) {
            showToast(getString(R.string.nodata));
            return;
        }
        List<CoinBaseTransferRecordEntity.TransactionsBean> transactions = transferRecordDetailEntity.getTransactions();
        if (transactions == null || transactions.size() == 0) {
            showToast(getString(R.string.nodata));
            return;
        }
        CoinBaseTransferRecordEntity.TransactionsBean transactionsBean = transactions.get(0);
        tvTransferamount.setText(NumberiUtil.maxNumberFormat(Arith.div(transactionsBean.getAmount() + "", MyWallet.RATE_S), 12) + " ELA");
        llCharge.setVisibility(View.GONE);
        llIn.setVisibility(View.GONE);//输入
        //输出
        llOut2.setVisibility(View.GONE);
        ivShowOut.setVisibility(View.GONE);
        llShowOut.setVisibility(View.GONE);
        tvAddressOut1.setText(transactionsBean.getAddress());
        tvAmountOut1.setVisibility(View.GONE);

        if (transactionsBean.getStatus().equals("Pending")) {
            tvSuretime.setText("- -");
        } else {
            tvSuretime.setText(DateUtil.time(transactionsBean.getTimestamp(), getContext()));
        }
        tvSuretimes.setText(transactionsBean.getConfirmStatus());
        tvRemark.setVisibility(View.GONE);

        int transferType;

        if (chainId.equals(MyWallet.IDChain)) {
            transferType = getContext().getResources().getIdentifier("sidetransfertype" + transactionsBean.getType(), "string",
                    getContext().getPackageName());
        } else {
            transferType = getContext().getResources().getIdentifier("transfertype" + transactionsBean.getType(), "string",
                    getContext().getPackageName());
        }
        try {
            tvType.setText(getString(transferType));
        } catch (Exception e) {
            tvType.setText(getString(R.string.transfertype13));
        }
    }
}
