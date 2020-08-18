package org.elastos.wallet.ela.ui.committee.fragment;

import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import com.google.gson.JsonObject;

import org.elastos.did.DIDStore;
import org.elastos.did.exception.DIDException;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.presenter.PwdPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.mulwallet.CreatMulWalletPresenter;
import org.elastos.wallet.ela.ui.committee.presenter.GetCRCDepositPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringWithiMethNameEntity;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalPresenter;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.json.JSONException;
import org.json.JSONObject;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class GetDepositFragment extends BaseFragment implements NewBaseViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_name)
    TextView tvName;
    @BindView(R.id.et_crcpk)
    EditText etCrcpk;
    @BindView(R.id.tv_next)
    TextView tvNext;
    Unbinder unbinder;
    private String did;
    private Wallet wallet;
    private GetCRCDepositPresenter getCRCDepositPresenter;
    private ProposalPresenter proposalPresenter;
    private String payPasswd;
    private DIDStore store;
    private JsonObject object;
    private String digest;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_getdeposit;
    }

    @Override
    protected void setExtraData(Bundle data) {
        String name = data.getString("name", "");
        String pk = data.getString("pk", "");
        if (!TextUtils.isEmpty(pk)) {
            tvTitle.setText(R.string.managecrnode);
            etCrcpk.setText(pk);
        } else {
            tvTitle.setText(R.string.getcrmode);
        }
        did = data.getString("did", "").replace("did:elastos:", "");
        tvName.setText("Node: " + name);
        wallet = data.getParcelable("wallet");
    }

    @Override
    protected void initView(View view) {

        getCRCDepositPresenter = new GetCRCDepositPresenter();
        proposalPresenter = new ProposalPresenter();
        registReceiver();
    }

    @OnClick({R.id.tv_next})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.tv_next:
                String pk = etCrcpk.getText().toString().trim();
                if (TextUtils.isEmpty(pk)) {
                    showToast(getString(R.string.plzinoutoperatepk));
                    return;
                }
                object = new JsonObject();
                object.addProperty("NodePublicKey", pk);
                object.addProperty("CRCouncilMemberDID", did);
                getCRCDepositPresenter.CRCouncilMemberClaimNodeDigest(wallet.getWalletId(), object.toString(), this);
                break;

        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "newPublishTransaction":

                String hash = "";
                try {
                    JSONObject pulishdata = new JSONObject(((CommmonStringWithiMethNameEntity) baseEntity).getData());
                    hash = pulishdata.getString("TxHash");
                } catch (JSONException e) {
                    e.printStackTrace();
                }
                post(RxEnum.TRANSFERSUCESS.ordinal(), 49 + "", hash);
                new DialogUtil().showTransferSucess(null, getBaseActivity(), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        pop();
                    }
                });


                break;
            case "signTransaction":
                new PwdPresenter().newPublishTransaction(wallet.getWalletId(), MyWallet.ELA, ((CommmonStringEntity) baseEntity).getData(), this);

                break;
            case "createCRCouncilMemberClaimNodeTransaction":
                new PwdPresenter().signTransaction(wallet.getWalletId(), MyWallet.ELA, ((CommmonStringEntity) baseEntity).getData(),payPasswd, this);

                break;
            case "CRCouncilMemberClaimNodeDigest":
                digest = ((CommmonStringEntity) baseEntity).getData();
                proposalPresenter.showFeePage(wallet, Constant.CRCDEPOSIT, 49, this, null);

                break;
            case "exportxPrivateKey":
                String privateKey = ((CommmonStringEntity) baseEntity).getData();
                try {
                    store.initPrivateIdentity(privateKey, payPasswd);
                    containsPrivateIdentity();
                } catch (DIDException e) {
                    e.printStackTrace();
                    showToast(getString(R.string.didinitfaile));
                }
                break;
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();

        if (integer == RxEnum.VERTIFYPAYPASS.ordinal()) {
            //验证密码成功
            if (getClass().getSimpleName().equals(result.getName())) {
                payPasswd = (String) result.getObj();
                initDid();
            }

        }
        if (integer == RxEnum.JUSTSHOWFEE.ordinal()) {
            //展示手续费后  再去验证密码
            if (getClass().getSimpleName().equals(result.getName())) {
                proposalPresenter.toVertifyPwdActivity(wallet, this);
            }

        }

    }

    private void initDid() {
        try {
            store = getMyDID().getDidStore();
            if (store.containsPrivateIdentity()) {
                containsPrivateIdentity();
            } else {
                //获得私钥用于初始化did
                new CreatMulWalletPresenter().exportxPrivateKey(wallet.getWalletId(), payPasswd, this);
            }
        } catch (DIDException e) {
            e.printStackTrace();
        }

    }

    private void containsPrivateIdentity() {
        getMyDID().initDID(payPasswd);
        String signDigest = new ProposalPresenter().getSignDigist(payPasswd, digest, this);
        object.addProperty("CRCouncilMemberSignature", signDigest);
        getCRCDepositPresenter.createCRCouncilMemberClaimNodeTransaction(wallet.getWalletId(), object.toString(), this);

    }

}
