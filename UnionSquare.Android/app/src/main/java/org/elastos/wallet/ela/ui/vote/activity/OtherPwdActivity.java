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
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.presenter.PwdPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.AndroidWorkaround;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 只为模拟交易获得手续费的情况准备
 */
public class OtherPwdActivity extends BaseActivity implements CommmonStringWithMethNameViewData, NewBaseViewData {
    @BindView(R.id.et_pwd)
    ClearEditText etPwd;
    private Wallet wallet;
    private String chainId, pwd;
    private PwdPresenter presenter;
    private String type, amount, nodePublicKey, ownerPublicKey, name, url;
    private long code;
    private String inputJson;
    private String did;

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
        inputJson = data.getStringExtra("inputJson");
        did = data.getStringExtra("did");


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
                switch (type) {
                    case Constant.SUPERNODESIGN:
                    case Constant.UPDATENODEINFO:
                        presenter.generateProducerPayload(wallet.getWalletId(), MyWallet.ELA, ownerPublicKey, nodePublicKey, name, url, "", code, pwd, this);
                        break;
                    case Constant.CRSIGNUP:
                    case Constant.CRUPDATE:
                        presenter.generateCRInfoPayload(wallet.getWalletId(), MyWallet.ELA, ownerPublicKey, name, url, code, pwd, this);
                        break;
                    case Constant.UNREGISTERSUPRRNODE:

                        presenter.generateCancelProducerPayload(wallet.getWalletId(), MyWallet.ELA, ownerPublicKey, pwd, this);
                        break;
                    case Constant.UNREGISTERCR:

                        presenter.generateUnregisterCRPayload(wallet.getWalletId(), MyWallet.ELA, did, pwd, this);
                        break;
                    case Constant.DIDSIGNUP:
                        presenter.generateDIDInfoPayload(wallet.getWalletId(), inputJson, pwd, this);
                        break;
                    case Constant.WITHDRAWSUPERNODE:
                        presenter.createRetrieveDepositTransaction(wallet.getWalletId(), MyWallet.ELA,
                                Arith.sub(Arith.mul(amount, MyWallet.RATE_S), "10000").toPlainString(), this);

                        break;
                    case Constant.WITHDRAWCR:
                        presenter.createRetrieveCRDepositTransaction(wallet.getWalletId(), MyWallet.ELA,
                                Arith.sub(Arith.mul(amount, MyWallet.RATE_S), "10000").toPlainString(), "", this);

                        break;

                }
                break;

        }
    }


    @Override
    public void onGetCommonData(String methodname, String data) {
        switch (methodname) {


            case "signTransaction":
                presenter.publishTransaction(wallet.getWalletId(), chainId, data, this);
                break;
            case "publishTransaction":
                post(RxEnum.TRANSFERSUCESS.ordinal(), getString(R.string.for_successful), null);
                finish();
                break;

        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

        switch (methodName) {
            case "generateDIDInfoPayload":
                presenter.createIDTransaction(wallet.getWalletId(), ((CommmonStringEntity) baseEntity).getData(), this);
                break;
            case "generateUnregisterCRPayload":
                presenter.createUnregisterCRTransaction(wallet.getWalletId(), MyWallet.ELA, "", ((CommmonStringEntity) baseEntity).getData(), this);
                break;
            case "generateCancelProducerPayload":
                //注销按钮
                presenter.createCancelProducerTransaction(wallet.getWalletId(), MyWallet.ELA, "", ((CommmonStringEntity) baseEntity).getData(), this);

                break;
            case "generateProducerPayload":
                if (type.equals(Constant.SUPERNODESIGN)) {
                    presenter.createRegisterProducerTransaction(wallet.getWalletId(), MyWallet.ELA, "", ((CommmonStringEntity) baseEntity).getData(), Arith.mul(amount, MyWallet.RATE_S).toPlainString(), "", true, this);
                } else if (type.equals(Constant.UPDATENODEINFO)) {
                    presenter.createUpdateProducerTransaction(wallet.getWalletId(), MyWallet.ELA, "", ((CommmonStringEntity) baseEntity).getData(), "", false, this);
                }
                break;


            //验证交易
            case "generateCRInfoPayload":
                if (type.equals(Constant.CRSIGNUP)) {
                    presenter.createRegisterCRTransaction(wallet.getWalletId(), MyWallet.ELA, "", ((CommmonStringEntity) baseEntity).getData(), Arith.mul("5000", MyWallet.RATE_S).toPlainString(), "", true, this);
                } else if (type.equals(Constant.CRUPDATE)) {
                    presenter.createUpdateCRTransaction(wallet.getWalletId(), MyWallet.ELA, "", ((CommmonStringEntity) baseEntity).getData(), "", false, this);
                }
                break;
            //创建交易
            case "createUpdateProducerTransaction":
            case "createRegisterProducerTransaction":
            case "createRegisterCRTransaction":
            case "createUpdateCRTransaction":
            case "createRetrieveDepositTransaction":
            case "createCancelProducerTransaction":
            case "createRetrieveCRDepositTransaction":
            case "createUnregisterCRTransaction":
            case "createIDTransaction":
                presenter.signTransaction(wallet.getWalletId(), chainId, ((CommmonStringEntity) baseEntity).getData(), pwd, this);
                break;
        }
    }
}


