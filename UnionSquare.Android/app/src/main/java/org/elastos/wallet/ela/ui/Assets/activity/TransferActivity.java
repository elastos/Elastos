package org.elastos.wallet.ela.ui.Assets.activity;

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
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.utils.AndroidWorkaround;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.json.JSONException;
import org.json.JSONObject;

import butterknife.BindView;
import butterknife.OnClick;


public class TransferActivity extends BaseActivity {


    @BindView(R.id.tv_address)
    TextView tvAddress;
    @BindView(R.id.tv_amount)
    TextView tvAmount;
    @BindView(R.id.tv_charge)
    TextView tvCharge;
    @BindView(R.id.ll_rate)
    LinearLayout llRate;
    @BindView(R.id.ll_address)
    LinearLayout llAddress;
    private Wallet wallet;
    private String chainId;
    private String amount;
    private long fee;
    private String toAddress;
    private String attributes;
    private String type;

    @Override
    protected int getLayoutId() {
        if (AndroidWorkaround.checkDeviceHasNavigationBar(this)) {
            AndroidWorkaround.assistActivity(findViewById(android.R.id.content));
        }
        return R.layout.activity_transfer;
    }

    @Override
    protected void initView() {
        getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        //一定要在setContentView之后调用，否则无效
        getWindow().setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        registReceiver();
    }

    @Override
    protected void setExtraData(Intent data) {

        chainId = data.getStringExtra("chainId");
        amount = data.getStringExtra("amount");
        toAddress = data.getStringExtra("toAddress");
        attributes = data.getStringExtra("attributes");

        try {
            JSONObject jsonObject = new JSONObject(attributes);

            if (jsonObject.has("Fee")) {
                fee = jsonObject.getLong("Fee");
            }

        } catch (JSONException e) {
            e.printStackTrace();
            fee = MyWallet.feePerKb;
        }
        wallet = data.getParcelableExtra("wallet");
        tvAddress.setText(toAddress);
        tvAmount.setText(amount + " ELA");
        //tvCharge.setText(NumberiUtil.maxNumberFormat(new BigDecimal(((double) fee) / MyWallet.RATE + "").toPlainString(), 12) + " " + chainId);//0.0001
        tvCharge.setText(NumberiUtil.maxNumberFormat(Arith.div(fee + "", MyWallet.RATE_S), 12) + " ELA");//0.0001


        type = data.getStringExtra("type");
        switch (type) {
            case Constant.SIDEWITHDRAW:
                //侧链充值
                break;
            case Constant.TRANFER:
                //转账
                llRate.setVisibility(View.GONE);
                break;
            case Constant.SUPERNODEVOTE:
                //转账
                llRate.setVisibility(View.GONE);
                llAddress.setVisibility(View.GONE);
                break;
        }
    }

    @OnClick({R.id.tv_next})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_next:
                //转账密码
                Intent intent;
                switch (wallet.getType()) {
                    //0 普通单签 1单签只读 2普通多签 3多签只读
                    case 0:
                    case 2:
                        intent = new Intent(this, PwdActivity.class);
                        intent.putExtra("wallet", wallet);
                        intent.putExtra("chainId", chainId);
                        intent.putExtra("attributes", attributes);
                        startActivity(intent);
                        break;
                    case 1:
                    case 3:
                        post(RxEnum.TOSIGN.ordinal(), "", attributes);
                        finish();
                        break;
                }

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
        if (integer == RxEnum.SIGNSUCCESS.ordinal()) {
            finish();

        }
    }

}
