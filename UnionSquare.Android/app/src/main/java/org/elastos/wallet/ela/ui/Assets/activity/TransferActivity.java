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

package org.elastos.wallet.ela.ui.Assets.activity;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
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


    @BindView(R.id.title)
    TextView title;
    @BindView(R.id.tv_amount_tag)
    TextView tvAmountTag;
    @BindView(R.id.tv_address)
    TextView tvAddress;
    @BindView(R.id.tv_amount)
    TextView tvAmount;
    @BindView(R.id.tv_charge)
    TextView tvCharge;
    @BindView(R.id.rl_rate)
    RelativeLayout rlRate;
    @BindView(R.id.rl_amount)
    RelativeLayout rlAmount;
    @BindView(R.id.rl_address)
    RelativeLayout rlAddress;
    @BindView(R.id.tv_hash)
    TextView tvHash;
    @BindView(R.id.rl_hash)
    RelativeLayout rlHash;
    @BindView(R.id.tv_rate)
    TextView tvRate;
    @BindView(R.id.tv_next)
    TextView tvNext;
    @BindView(R.id.ll)
    LinearLayout ll;
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
        amount = data.getStringExtra("amount");
        if ("MAX".equals(amount)) {
            tvAmount.setText("MAX");
        } else {
            tvAmount.setText(NumberiUtil.numberFormat(amount, 8) + " ELA");
        }
        wallet = data.getParcelableExtra("wallet");
        tvAddress.setText(toAddress);

        //tvCharge.setText(NumberiUtil.maxNumberFormat(new BigDecimal(((double) fee) / MyWallet.RATE + "").toPlainString(), 12) + " " + chainId);//0.0001
        tvCharge.setText(NumberiUtil.maxNumberFormat(Arith.div(fee + "", MyWallet.RATE_S), 12) + " ELA");//0.0001


        type = data.getStringExtra("type");
        switch (type) {
            case Constant.IMPEACHMENTCRC:
                rlAddress.setVisibility(View.GONE);
                rlRate.setVisibility(View.GONE);
                title.setText(R.string.impeachment);
                tvAmountTag.setText(R.string.impeachvotes);
                break;
            case Constant.SIDEWITHDRAW:
                //侧链充值
                break;
            case Constant.TRANFER:
                //转账
                rlRate.setVisibility(View.GONE);
                break;
            case Constant.PROPOSALPUBLISHED:
                //公示期投票
                String hash = data.getStringExtra("extra");
                rlHash.setVisibility(View.VISIBLE);
                tvHash.setText(hash);
                tvAmountTag.setText(R.string.rejectticket);
            case Constant.SUPERNODEVOTE:
            case Constant.CRVOTE:
            case Constant.WITHDRAWSUPERNODE:
            case Constant.WITHDRAWCR:

                //超级节点投票  cr投票
                rlRate.setVisibility(View.GONE);
                rlAddress.setVisibility(View.GONE);
                break;
            case Constant.TOWHOL:
                //超级节点投票  cr投票
                rlAmount.setVisibility(View.GONE);
                rlRate.setVisibility(View.GONE);
                rlAddress.setVisibility(View.GONE);
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
                        intent.putExtras(getIntent());
                      /*  intent.putExtra("wallet", wallet);
                        intent.putExtra("chainId", chainId);
                        intent.putExtra("attributes", attributes);*/
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
