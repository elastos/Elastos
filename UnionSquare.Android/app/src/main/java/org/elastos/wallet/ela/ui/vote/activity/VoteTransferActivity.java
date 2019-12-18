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
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.utils.AndroidWorkaround;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 只为模拟交易获得手续费的情况准备
 */
public class VoteTransferActivity extends BaseActivity {


    @BindView(R.id.tv_address)
    TextView tvAddress;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_amount)
    TextView tvAmount;
    @BindView(R.id.tv_charge)
    TextView tvCharge;
    @BindView(R.id.ll_amount)
    LinearLayout llAmount;
    private String amount, chainId;
    private long fee;

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
        registReceiver();
    }

    Intent intent;

    @Override
    protected void setExtraData(Intent data) {
        this.intent = data;
        amount = data.getStringExtra("amount");
        fee = data.getLongExtra("fee", MyWallet.feePerKb);
        chainId = data.getStringExtra("chainId");
        if (chainId == null) {
            chainId = MyWallet.ELA;
        }
        tvAmount.setText(amount + " " + MyWallet.ELA);
        tvCharge.setText(NumberiUtil.maxNumberFormat(Arith.div(fee + "", MyWallet.RATE_S).toPlainString(), 12) + " " + MyWallet.ELA);//0.0001
        String type = data.getStringExtra("type");
        switch (type) {
            case Constant.CRUPDATE:
            case Constant.UPDATENODEINFO:
            case Constant.UNREGISTERSUPRRNODE:
            case Constant.UNREGISTERCR:
            case Constant.DIDSIGNUP:
                llAmount.setVisibility(View.GONE);

                break;
            case Constant.CRSIGNUP:
            case Constant.SUPERNODESIGN:
                tvTitle.setText(getString(R.string.electiondeposit));

                break;

        }
    }

    @OnClick({R.id.tv_next})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_next:
                Intent intent = new Intent(this, OtherPwdActivity.class);
                intent.putExtras(this.intent);
                startActivity(intent);
                break;

        }
    }

    //TRANSFERSUCESS
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            finish();

        }

    }
}
