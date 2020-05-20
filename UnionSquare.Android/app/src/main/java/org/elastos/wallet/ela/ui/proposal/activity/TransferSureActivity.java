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

package org.elastos.wallet.ela.ui.proposal.activity;

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
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecieveReviewJwtEntity;
import org.elastos.wallet.ela.ui.vote.activity.OtherPwdActivity;
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
 * 只为确认交易
 */
public class TransferSureActivity extends BaseActivity {


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
    @BindView(R.id.line1)
    View line1;
    @BindView(R.id.tv_hash_tag)
    TextView tvHashTag;
    @BindView(R.id.tv_hash)
    TextView tvHash;
    @BindView(R.id.rl_hash)
    RelativeLayout rlHash;
    @BindView(R.id.tv_voteadvice)
    TextView tvVoteadvice;
    @BindView(R.id.rl_voteadvice)
    RelativeLayout rlVoteadvice;
    @BindView(R.id.tv_next)
    TextView tvNext;
    @BindView(R.id.ll)
    LinearLayout ll;
    private String amount, chainId;
    private long fee;
    private String type;

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


    @Override
    protected void setExtraData(Intent data) {

        amount = data.getStringExtra("amount");
        fee = data.getLongExtra("fee", MyWallet.feePerKb);
        chainId = data.getStringExtra("chainId");
        if (chainId == null) {
            chainId = MyWallet.ELA;
        }
        tvAmount.setText(amount + " " + MyWallet.ELA);
     /*   if (MyWallet.IDChain.equals(chainId)) {
            tvCharge.setText("0.0002 " + MyWallet.ELA);
        } else {*/
        tvCharge.setText(NumberiUtil.maxNumberFormat(Arith.div(fee + "", MyWallet.RATE_S).toPlainString(), 12) + " " + MyWallet.ELA);//0.0001
        //   }
        type = data.getStringExtra("type");
        switch (type) {
            case Constant.PROPOSALREVIEW:
                doProposalReview(data);
            case Constant.CRUPDATE:
            case Constant.UPDATENODEINFO:
            case Constant.UNREGISTERSUPRRNODE:
            case Constant.UNREGISTERCR:
            case Constant.DIDSIGNUP:
            case Constant.DIDUPDEATE:
            case Constant.PROPOSALINPUT:
                llAmount.setVisibility(View.GONE);
                line1.setVisibility(View.GONE);
                break;
            case Constant.CRSIGNUP:
            case Constant.SUPERNODESIGN:
                tvTitle.setText(getString(R.string.electiondeposit));

                break;

        }
    }

    private void doProposalReview(Intent data) {
        rlHash.setVisibility(View.VISIBLE);
        RecieveReviewJwtEntity.DataBean dataBean = data.getParcelableExtra("extra");
        tvHash.setText(dataBean.getOpinionhash());
        rlVoteadvice.setVisibility(View.VISIBLE);
        switch (dataBean.getVoteresult().toLowerCase()) {
            case "approve":
                tvVoteadvice.setText(R.string.agree1);
                tvVoteadvice.setBackgroundResource(R.drawable.sc_35b08f_cr3);
                break;
            case "reject":
                tvVoteadvice.setText(R.string.disagree1);
                tvVoteadvice.setBackgroundResource(R.drawable.sc_b04135_cr3);
                break;
            case "abstain":
                tvVoteadvice.setText(R.string.abstention);
                tvVoteadvice.setBackgroundResource(R.drawable.sc_ffffff_sc000000_cr3);
                break;
        }
    }

    @OnClick({R.id.tv_next})
    public void onViewClicked(View view) {
        Intent intent;
        switch (view.getId()) {
            case R.id.tv_next:
                if (Constant.PROPOSALINPUT.equals(type) || Constant.PROPOSALREVIEW.equals(type)) {
                    //为了在展示手续费后把密码返回给fragment
                  /*  intent = new Intent(this, VertifyPwdActivity.class);
                    intent.putExtra("walletId", chainId);
                    intent.putExtra("type", type);
                    startActivity(intent);*/
                    post(RxEnum.JUSTSHOWFEE.ordinal(), null, null);
                } else {
                    intent = new Intent(this, OtherPwdActivity.class);
                    intent.putExtras(getIntent());
                    startActivity(intent);
                }
                break;

        }
    }

    //TRANSFERSUCESS
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.TRANSFERSUCESS.ordinal() || integer == RxEnum.VERTIFYPAYPASS.ordinal()) {
            finish();

        }

    }
}
