package org.elastos.wallet.ela.ui.Assets.activity;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.presenter.PwdPresenter;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.AndroidWorkaround;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;

public class PwdActivity extends BaseActivity implements CommmonStringWithMethNameViewData {
    @BindView(R.id.et_pwd)
    ClearEditText etPwd;
    private Wallet wallet;
    private String chainId;
    private PwdPresenter presenter;
    private String attributes;
    private String pwd;
    private int type;

    @Override
    protected int getLayoutId() {
        if (AndroidWorkaround.checkDeviceHasNavigationBar(this)) {
            AndroidWorkaround.assistActivity(findViewById(android.R.id.content));
        }
        return R.layout.activity_pwd;
    }

    @Override
    protected void initView() {

        getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        //一定要在setContentView之后调用，否则无效
        getWindow().setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        presenter = new PwdPresenter();
    }

    @Override
    protected void setExtraData(Intent data) {
        chainId = data.getStringExtra("chainId");
        attributes = data.getStringExtra("attributes");
        wallet = data.getParcelableExtra("wallet");
        type = data.getIntExtra("type", 0);//1只签名目前只有钱包管理的签名用到
      /*  tvAddress.setText(toAddress);
        tvAmount.setText(amount + " " + chainId);
        tvCharge.setText(NumberiUtil.maxNumberFormat(new BigDecimal(((double) fee) / MyWallet.RATE + "").toPlainString(), 12) + " " + chainId);//0.0001
        llRate.setVisibility(View.GONE);*/
    }

    @OnClick({R.id.tv_sure})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sure:
                //确定
                pwd = etPwd.getText().toString().trim();
                if (TextUtils.isEmpty(pwd)) {
                    showToastMessage(getString(R.string.pwdnoempty));
                    return;
                }
                presenter.signTransaction(wallet.getWalletId(), chainId, attributes, pwd, this);
                break;

        }
    }


    @Override
    public void onGetCommonData(String methodname, String data) {
        switch (methodname) {
            case "signTransaction":
                if (type == 1) {
                    post(RxEnum.SIGNSUCCESS.ordinal(), "签名成功", data);
                    finish();
                    return;
                }
                presenter.publishTransaction(wallet.getWalletId(), chainId, data, this);
                break;
            case "publishTransaction":
                post(RxEnum.TRANSFERSUCESS.ordinal(), "转账成功", null);
                finish();
                break;
        }
    }
}
