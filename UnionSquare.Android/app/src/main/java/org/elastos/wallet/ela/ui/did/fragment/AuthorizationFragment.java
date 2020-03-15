package org.elastos.wallet.ela.ui.did.fragment;

import android.content.Intent;
import android.graphics.drawable.PictureDrawable;
import android.os.Bundle;
import android.util.Base64;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.did.DIDStore;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.bean.CallBackJwtEntity;
import org.elastos.wallet.ela.ui.Assets.bean.RecieveJwtEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.mulwallet.CreatMulWalletPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.did.presenter.AuthorizationPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VertifyPwdActivity;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.svg.GlideApp;
import org.elastos.wallet.ela.utils.svg.SvgSoftwareLayerSetter;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.Date;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class AuthorizationFragment extends BaseFragment implements NewBaseViewData {
    @BindView(R.id.iv_logo)
    ImageView ivLogo;
    @BindView(R.id.tv_name)
    TextView tvName;
    @BindView(R.id.tv_did)
    TextView tvDid;
    Unbinder unbinder;
    private String[] jwtParts;
    RecieveJwtEntity recieveJwtEntity;
    private String scanResult;
    private String payPasswd;
    String callBackUrl, walletId;
    private DIDStore store;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_did_authorization;
    }


    @Override
    protected void setExtraData(Bundle data) {
        scanResult = data.getString("scanResult");
        walletId = data.getString("walletId");
        String result = scanResult.replace("elastos://credaccess/", "");
        jwtParts = result.split("\\.");
        String payload = new String(Base64.decode(jwtParts[1], Base64.URL_SAFE));
        recieveJwtEntity = JSON.parseObject(payload, RecieveJwtEntity.class);
        tvDid.setText(recieveJwtEntity.getIss());
        tvName.setText(recieveJwtEntity.getWebsite().getDomain());

        String logo = recieveJwtEntity.getWebsite().getLogo();
        if (logo.endsWith(".svg")) {
            GlideApp.with(this).as(PictureDrawable.class).listener(new SvgSoftwareLayerSetter()).load(logo).into(ivLogo);

        } else {
            GlideApp.with(this).load(logo).into(ivLogo);
        }
        registReceiver();
    }

    @Override
    protected void initView(View view) {
        callBackUrl = recieveJwtEntity.getCallbackurl();


    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "postData":
                showToast("授权成功");
                popBackFragment();
                break;
            case "exportxPrivateKey":
                String privateKey = ((CommmonStringEntity) baseEntity).getData();
                String payPasswd = (String) o;
                try {
                    store.initPrivateIdentity(privateKey, payPasswd);
                    generBackJwt();
                } catch (DIDException e) {
                    e.printStackTrace();
                    showToast(getString(R.string.didinitfaile));
                }
                break;
        }
    }

    @OnClick({R.id.tv_agree, R.id.tv_refuse, R.id.iv_logo, R.id.tv_name, R.id.tv_did})
    public void onViewClicked(View view) {

        switch (view.getId()) {
            case R.id.iv_logo:
            case R.id.tv_name:
            case R.id.tv_did:
                ClipboardUtil.copyClipboar(getBaseActivity(), tvDid.getText().toString());
                break;
            case R.id.tv_agree:
                Intent intent = new Intent(getActivity(), VertifyPwdActivity.class);
                intent.putExtra("walletId", walletId);
                intent.putExtra("type", this.getClass().getSimpleName());
                startActivity(intent);


                break;
            case R.id.tv_refuse:

                popBackFragment();
                break;


        }
    }

    private void generBackJwt() {
        getMyDID().initDID(payPasswd);
        CallBackJwtEntity callBackJwtEntity = new CallBackJwtEntity();
        callBackJwtEntity.setType("credaccess");
        callBackJwtEntity.setIss(getMyDID().getDidString());
        callBackJwtEntity.setIat(new Date().getTime());
        callBackJwtEntity.setExp(new Date().getTime() + 5 * 60 * 1000);
        callBackJwtEntity.setAud(recieveJwtEntity.getIss());
        callBackJwtEntity.setReq(scanResult);
        callBackJwtEntity.setPresentation("");
        String header = jwtParts[0];
        // Base64
        String payload = Base64.encodeToString(JSON.toJSONString(callBackJwtEntity).getBytes(), Base64.URL_SAFE|Base64.NO_WRAP);
        payload = payload.replaceAll("=", "");
        try {
            String signature = getMyDID().getDIDDocument().sign(payPasswd, (header + "." + payload).getBytes());
            new AuthorizationPresenter().postData(callBackUrl, header + "." + payload + "." + signature, this);
        } catch (DIDStoreException e) {
            e.printStackTrace();
        }

    }

    private void initDid(String payPasswd) {

        try {
            store = getMyDID().getDidStore();
            if (store.containsPrivateIdentity()) {
                generBackJwt();
            } else {
                //获得私钥用于初始化did
                new CreatMulWalletPresenter().exportxPrivateKey(walletId, payPasswd, this);
            }
        } catch (DIDException e) {
            e.printStackTrace();
        }

    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();

        if (integer == RxEnum.VERTIFYPAYPASS.ordinal()) {
            //验证密码成功
            payPasswd = (String) result.getObj();
            initDid((String) result.getObj());
        }

    }
}
