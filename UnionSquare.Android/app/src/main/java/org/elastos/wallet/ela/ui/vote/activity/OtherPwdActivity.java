package org.elastos.wallet.ela.ui.vote.activity;

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
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;

public class OtherPwdActivity extends BaseActivity implements CommmonStringWithMethNameViewData {
    @BindView(R.id.et_pwd)
    ClearEditText etPwd;
    private Wallet wallet;
    private String chainId, pwd;
    private PwdPresenter presenter;
    private String type, amount, nodePublicKey, ownerPublicKey, name, url;
    private long code;

    @Override
    protected int getLayoutId() {
        if (AndroidWorkaround.checkDeviceHasNavigationBar(this)) {
            AndroidWorkaround.assistActivity(findViewById(android.R.id.content));
        }
        return R.layout.activity_other_pwd;
    }

    @Override
    protected void initView() {

        getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        //一定要在setContentView之后调用，否则无效
        getWindow().setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);

    }

    @Override
    protected void setExtraData(Intent data) {

        wallet = data.getParcelableExtra("wallet");
        type = data.getStringExtra("type");
        amount = data.getStringExtra("amount");
        chainId = data.getStringExtra("chainId");
        nodePublicKey = data.getStringExtra("nodePublicKey");
        ownerPublicKey = data.getStringExtra("ownerPublicKey");
        name = data.getStringExtra("name");
        url = data.getStringExtra("url");
        code = data.getLongExtra("code", 0);


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
                presenter = new PwdPresenter();
                presenter.generateProducerPayload(wallet.getWalletId(), MyWallet.ELA, ownerPublicKey, nodePublicKey, name, url, "", code, pwd, this);
                break;

        }
    }


    @Override
    public void onGetCommonData(String methodname, String data) {
        switch (methodname) {
            case "generateProducerPayload":
                presenter.createRegisterProducerTransaction(wallet.getWalletId(), MyWallet.ELA, "", data, Arith.mul(amount, MyWallet.RATE_S).toPlainString(), "", true, this);

                break;
            case "createRegisterProducerTransaction":
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


