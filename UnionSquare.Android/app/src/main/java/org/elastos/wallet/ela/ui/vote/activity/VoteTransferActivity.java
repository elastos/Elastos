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

package org.elastos.wallet.ela.ui.vote.activity;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecieveProcessJwtEntity;
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecieveReviewJwtEntity;
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecieveWithdrawJwtEntity;
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
 * 只为模拟交易获得手续费和固定手续费的情况准备
 */
public class VoteTransferActivity extends BaseActivity {


    @BindView(R.id.base_title_left_pic)
    ImageView baseTitleLeftPic;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_address)
    TextView tvAddress;
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
    @BindView(R.id.tv_amount)
    TextView tvAmount;
    @BindView(R.id.ll_amount)
    RelativeLayout llAmount;
    @BindView(R.id.tv_charge)
    TextView tvCharge;
    @BindView(R.id.rl_fee)
    RelativeLayout rlFee;
    @BindView(R.id.tv_next)
    TextView tvNext;
    @BindView(R.id.ll)
    LinearLayout ll;
    @BindView(R.id.tv_address_tag)
    TextView tvAddressTag;
    @BindView(R.id.rl_address)
    RelativeLayout rlAddress;
    private String amount, chainId;
    private long fee;
    private String type;
    private String openType;//区分谁打开的classsimplename

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
        openType=data.getStringExtra("openType");
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
            case Constant.PROPOSALWITHDRAW:
                doProposalWithDraw(data);
                break;
            case Constant.PROPOSALSECRET:
                doProposalSecret(data);
                break;
            case Constant.PROPOSALPROCESS:
                doProposalProcess(data);

                break;
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

                break;
            case Constant.CRSIGNUP:
            case Constant.SUPERNODESIGN:
                tvTitle.setText(getString(R.string.electiondeposit));

                break;

        }
    }

    private void doProposalProcess(Intent data) {
        llAmount.setVisibility(View.GONE);
        rlFee.setVisibility(View.GONE);
        tvTitle.setText(R.string.suremessage);
        rlHash.setVisibility(View.VISIBLE);
        tvHashTag.setText(R.string.feedbackhash);
        RecieveProcessJwtEntity.DataBean dataBean = data.getParcelableExtra("extra");
        tvHash.setText(dataBean.getMessagehash());

    }

    private void doProposalWithDraw(Intent data) {
        rlAddress.setVisibility(View.VISIBLE);
        tvAddressTag.setText(R.string.withdrawaddress);
        RecieveWithdrawJwtEntity.DataBean dataBean = data.getParcelableExtra("extra");
        tvAddress.setText(dataBean.getRecipient());
        tvAmount.setText(NumberiUtil.salaToEla(dataBean.getAmount()) + " " + MyWallet.ELA);

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

    private void doProposalSecret(Intent data) {
        llAmount.setVisibility(View.GONE);
        tvTitle.setText(R.string.suremessage);
        rlHash.setVisibility(View.VISIBLE);
        tvHashTag.setText(R.string.optionhash);
        RecieveProcessJwtEntity.DataBean dataBean = data.getParcelableExtra("extra");
        tvHash.setText(dataBean.getSecretaryopinionhash());
        rlVoteadvice.setVisibility(View.VISIBLE);
        switch (dataBean.getProposaltrackingtype().toLowerCase()) {
            case "progress":
            case "finalized":
                tvVoteadvice.setText(R.string.pass);
                tvVoteadvice.setBackgroundResource(R.drawable.sc_35b08f_cr3);
                break;
            case "rejected":
                tvVoteadvice.setText(R.string.rejected);
                tvVoteadvice.setBackgroundResource(R.drawable.sc_b04135_cr3);
                break;

        }
    }

    @OnClick({R.id.tv_next})
    public void onViewClicked(View view) {
        Intent intent;
        switch (view.getId()) {
            case R.id.tv_next:
                if (Constant.PROPOSALINPUT.equals(type) || Constant.PROPOSALREVIEW.equals(type)
                        || Constant.PROPOSALPROCESS.equals(type) || Constant.PROPOSALSECRET.equals(type)
                        || Constant.PROPOSALWITHDRAW.equals(type)) {
                    //为了在展示手续费后把密码返回给fragment
                  /*  intent = new Intent(this, VertifyPwdActivity.class);
                    intent.putExtra("walletId", chainId);
                    intent.putExtra("type", type);
                    startActivity(intent);*/
                    post(RxEnum.JUSTSHOWFEE.ordinal(), openType, null);
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
