package org.elastos.wallet.ela.ui.vote.activity;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.presenter.PwdPresenter;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.AndroidWorkaround;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;


public class VoteTransferActivity extends BaseActivity implements CommmonStringWithMethNameViewData {


    @BindView(R.id.tv_address)
    TextView tvAddress;
    @BindView(R.id.tv_amount)
    TextView tvAmount;
    @BindView(R.id.tv_charge)
    TextView tvCharge;
    @BindView(R.id.ll_rate)
    LinearLayout llRate;
    private Wallet wallet;
    private String chainId;
    private String amount;
    private long fee;
    private String toAddress;
    private String attributes;
    private String type;
    private PwdPresenter presenter;
    private String pwd;

    @Override
    protected int getLayoutId() {
        if (AndroidWorkaround.checkDeviceHasNavigationBar(this)) {
            AndroidWorkaround.assistActivity(findViewById(android.R.id.content));
        }
        return R.layout.activity_vote_transfer;
    }

    @Override
    protected void initView() {
        getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        //一定要在setContentView之后调用，否则无效
        getWindow().setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);

    }

    @Override
    protected void setExtraData(Intent data) {

        chainId = data.getStringExtra("chainId");
        amount = data.getStringExtra("amount");
        toAddress = data.getStringExtra("toAddress");
        attributes = data.getStringExtra("attributes");
        fee = data.getLongExtra("fee", MyWallet.feePerKb / MyWallet.RATE);
        wallet = data.getParcelableExtra("wallet");
        pwd = data.getStringExtra("pwd");
        tvAddress.setText(toAddress);
        tvAmount.setText(amount + " " + chainId);
        //tvCharge.setText(NumberiUtil.maxNumberFormat(new BigDecimal(((double) fee) / MyWallet.RATE + "").toPlainString(), 12) + " " + chainId);//0.0001
        tvCharge.setText(NumberiUtil.maxNumberFormat(Arith.div(fee+"",MyWallet.RATE_S).toPlainString(), 12) + " " + chainId);//0.0001
        presenter = new PwdPresenter();


        type = data.getStringExtra("type");
        switch (type) {
            case Constant.SIDEWITHDRAW:
                //侧链充值
                break;
            case Constant.TRANFER:
                //转账
                llRate.setVisibility(View.GONE);
                break;

        }


    }

    @OnClick({R.id.tv_next})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_next:
                //更新手续费
                presenter.updateTransactionFee(wallet.getWalletId(), chainId, attributes, fee, "", this);
                break;

        }
    }


    @Override
    public void onGetCommonData(String methodname, String data) {
        switch (methodname) {

            case "updateTransactionFee":
                presenter.signTransaction(wallet.getWalletId(), chainId, data, pwd, this);
                break;
            case "signTransaction":
                presenter.publishTransaction(wallet.getWalletId(), chainId, data, this);
                break;
            case "publishTransaction":
                post(RxEnum.TRANSFERSUCESS.ordinal(), getString(R.string.for_successful), null);
                finish();
                break;
        }
    }
}
