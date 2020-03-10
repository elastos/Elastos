package org.elastos.wallet.ela.ui.vote.activity;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Looper;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONException;
import com.alibaba.fastjson.JSONObject;

import org.elastos.did.DIDAdapter;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.DID.listener.MyDIDTransactionCallback;
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
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;

import java.util.Date;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 只为模拟交易获得手续费的情况准备 和init did
 */
public class OtherPwdActivity extends BaseActivity implements CommmonStringWithMethNameViewData, NewBaseViewData, MyDIDTransactionCallback {
    @BindView(R.id.et_pwd)
    ClearEditText etPwd;
    private Wallet wallet;
    private String chainId, pwd;
    private PwdPresenter presenter;
    private String type, amount, nodePublicKey, ownerPublicKey, name, url;
    private long code;
    //private String inputJson;
    private String CID;
    private int transType;
    private JSONObject paylodJson;
    private String didName;
    private Date didEndDate;

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
        if (chainId == null) {
            chainId = MyWallet.ELA;
        }
        nodePublicKey = data.getStringExtra("nodePublicKey");
        ownerPublicKey = data.getStringExtra("ownerPublicKey");
        name = data.getStringExtra("name");
        url = data.getStringExtra("url");
        code = data.getLongExtra("code", 0);
        //inputJson = data.getStringExtra("inputJson");
        CID = data.getStringExtra("CID");
        didName = data.getStringExtra("didName");
        didEndDate = (Date) data.getSerializableExtra("didEndDate");

        transType = data.getIntExtra("transType", 13);


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
                        presenter.generateCRInfoPayload(wallet.getWalletId(), MyWallet.ELA, ownerPublicKey, name, url, code, this);
                        break;
                    case Constant.UNREGISTERSUPRRNODE:

                        presenter.generateCancelProducerPayload(wallet.getWalletId(), MyWallet.ELA, ownerPublicKey, pwd, this);
                        break;
                    case Constant.UNREGISTERCR:

                        presenter.generateUnregisterCRPayload(wallet.getWalletId(), MyWallet.ELA, CID, this);
                        break;
                    case Constant.DIDSIGNUP:
                        getMyDID().setDIDDocumentExprise(didEndDate,pwd , didName);
                        getMyDID().getMyDIDAdapter().setMyDIDTransactionCallback(this);
                        presenter.DIDPublish(pwd, this);
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
                String hash = "";
                try {
                    JSONObject pulishdata = JSON.parseObject(data);
                    hash = pulishdata.getString("TxHash");
                    getMyDID().getMyDIDAdapter().setTxId(hash);
                } catch (JSONException e) {
                    e.printStackTrace();
                }
                post(RxEnum.TRANSFERSUCESS.ordinal(), transType + "", hash);
                finish();
                break;

        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

        switch (methodName) {
            case "signDigest":
                paylodJson.put("Signature", ((CommmonStringEntity) baseEntity).getData());
                if (type.equals(Constant.CRSIGNUP)) {
                    presenter.createRegisterCRTransaction(wallet.getWalletId(), MyWallet.ELA, "", paylodJson.toString(), Arith.mulRemoveZero("5000", MyWallet.RATE_S).toPlainString(), "", true, this);
                } else if (type.equals(Constant.CRUPDATE)) {
                    presenter.createUpdateCRTransaction(wallet.getWalletId(), MyWallet.ELA, "", paylodJson.toString(), "", false, this);
                } else if (type.equals(Constant.UNREGISTERCR)) {
                    presenter.createUnregisterCRTransaction(wallet.getWalletId(), MyWallet.ELA, "", paylodJson.toString(), this);
                }
                break;
            case "generateDIDInfoPayload":
                presenter.createIDTransaction(wallet.getWalletId(), ((CommmonStringEntity) baseEntity).getData(), this);
                break;

            case "generateCancelProducerPayload":
                //注销按钮
                presenter.createCancelProducerTransaction(wallet.getWalletId(), MyWallet.ELA, "", ((CommmonStringEntity) baseEntity).getData(), this);

                break;
            case "generateProducerPayload":
                if (type.equals(Constant.SUPERNODESIGN)) {
                    presenter.createRegisterProducerTransaction(wallet.getWalletId(), MyWallet.ELA, "", ((CommmonStringEntity) baseEntity).getData(), Arith.mulRemoveZero(amount, MyWallet.RATE_S).toPlainString(), "", true, this);
                } else if (type.equals(Constant.UPDATENODEINFO)) {
                    presenter.createUpdateProducerTransaction(wallet.getWalletId(), MyWallet.ELA, "", ((CommmonStringEntity) baseEntity).getData(), "", false, this);
                }
                break;


            //验证交易
            case "generateCRInfoPayload":
            case "generateUnregisterCRPayload":
                String payload = ((CommmonStringEntity) baseEntity).getData();
                paylodJson = JSON.parseObject(payload);
                String digest = paylodJson.getString("Digest");
                if (type.equals(Constant.CRSIGNUP) || type.equals(Constant.CRUPDATE) || type.equals(Constant.UNREGISTERCR)) {
                    presenter.signDigest(wallet.getWalletId(), CID, digest, pwd, this);
                }
                break;
            //创建交易
            case "createUpdateProducerTransaction":
            case "createRegisterProducerTransaction":
            case "createRegisterCRTransaction":
            case "createUpdateCRTransaction":
            case "createCancelProducerTransaction":
            case "createUnregisterCRTransaction":
            case "createIDTransaction":
                presenter.signTransaction(wallet.getWalletId(), chainId, ((CommmonStringEntity) baseEntity).getData(), pwd, this);
                break;
        }
    }

    @Override
    public void createIdTransaction(String payload, String memo, int confirms, DIDAdapter.TransactionCallback callback) {
        Looper.prepare();
        presenter.createIDTransaction(wallet.getWalletId(), payload, this);
        Looper.loop();// 进入loop中的循环，查看消息队列


    }
}


