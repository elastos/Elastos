package org.elastos.wallet.ela.ui.did.fragment;

import android.content.Intent;
import android.graphics.drawable.PictureDrawable;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Base64;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.did.DIDDocument;
import org.elastos.did.DIDStore;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.bean.CallBackJwtEntity;
import org.elastos.wallet.ela.ui.Assets.bean.RecieveJwtEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.mulwallet.CreatMulWalletPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonLongEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.crvote.bean.CrStatusBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.did.adapter.AuthorizationRecAdapetr;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.entity.PersonalInfoItemEntity;
import org.elastos.wallet.ela.ui.did.presenter.AuthorizationPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VertifyPwdActivity;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.svg.GlideApp;
import org.elastos.wallet.ela.utils.svg.SvgSoftwareLayerSetter;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
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
    @BindView(R.id.rv)
    RecyclerView rv;
    Unbinder unbinder;
    private String[] jwtParts;
    RecieveJwtEntity recieveJwtEntity;
    private String scanResult;
    String callBackUrl;
    private DIDStore store;
    private String type;
    private Wallet wallet;
    private String name;
    private int code;
    private String url;
    private String ownerPublicKey;
    private String CID;
    private ArrayList<PersonalInfoItemEntity> listShow;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_did_authorization;
    }

    private void initAuthorization() {
        ivLogo.setImageResource(R.mipmap.found_cr_vote);
        tvName.setText(R.string.findlistup4);
        tvDid.setVisibility(View.GONE);
    }

    @Override
    protected void setExtraData(Bundle data) {
        wallet = data.getParcelable("wallet");
        type = data.getString("type");
        if ("authorization".equals(type)) {
            //授权同时绑定did
            initItemDate();
            initAuthorization();
        } else if ("authorization&bind".equals(type)) {
            initItemDate();
            initAuthorization();
            CrStatusBean crStatusBean = data.getParcelable("crStatusBean");
            CrStatusBean.InfoBean bean = crStatusBean.getInfo();
            name = bean.getNickName();
            code = bean.getLocation();
            url = bean.getURL();
            ownerPublicKey = bean.getCROwnerPublicKey();
            CID = bean.getCID();
        } else {
            //扫描授权
            scanResult = data.getString("scanResult");

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
            callBackUrl = recieveJwtEntity.getCallbackurl();
        }
    }

    @Override
    protected void initView(View view) {
        registReceiver();
    }

    private void initItemDate() {

        String showData[] = getResources().getStringArray(R.array.personalinfo_chose);
        /*  Map<Integer, String>*/
        listShow = new ArrayList<>();

        for (int i = 0; i < showData.length; i++) {
            if (i == 1 || i == 2 || i == 3 || i == 4 || i == 7 || i == 8 || i == 9 || i == 10 || i == 11 || i == 12 || i == 13) {
                PersonalInfoItemEntity personalInfoItemEntity = new PersonalInfoItemEntity();
                personalInfoItemEntity.setIndex(i);
                personalInfoItemEntity.setHintShow1("-" + showData[i]);
                personalInfoItemEntity.setCheck(true);
                listShow.add(personalInfoItemEntity);
            }

        }

        setRecycleViewShow();
    }

    private void setRecycleViewShow() {

        AuthorizationRecAdapetr adapterShow = new AuthorizationRecAdapetr(getContext(), listShow);
        rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rv.setAdapter(adapterShow);

    }

    private CredentialSubjectBean convertCredentialSubjectBean(DIDDocument doc) {
        //这种情况考虑去除全局变量credentialSubjectBean
        String json = getMyDID().getCredentialProFromStore(getMyDID().getDidString());
        CredentialSubjectBean result = JSON.parseObject(json, CredentialSubjectBean.class);
        if (result == null) {
            result = new CredentialSubjectBean(getMyDID().getDidString(), getMyDID().getName(doc));
            return result;
        } else {
            result.setDidName(getMyDID().getName(doc));//单独更新姓名
        }
        for (int i = 0; i < listShow.size(); i++) {
            //只遍历show的数据
            PersonalInfoItemEntity personalInfoItemEntity = listShow.get(i);
            int index = personalInfoItemEntity.getIndex();
            boolean check = personalInfoItemEntity.isCheck();
            switch (index) {
                case 0:
                    if (!check)
                        result.setNickname(null);
                    break;
                case 1:
                    if (!check)
                        result.setGender(null);
                    break;
                case 2:
                    if (!check)

                        result.setBirthday(null);
                    break;
                case 3:
                    if (!check)
                        result.setAvatar(null);
                    break;
                case 4:
                    if (!check)
                        result.setEmail(null);
                    break;
                case 5:
                    if (!check) {
                        result.setPhoneCode(null);
                        result.setPhone(null);
                    }
                    break;
                case 6:
                    if (!check)
                        result.setNation(null);
                    break;
                case 7:
                    if (!check)
                        result.setIntroduction(null);

                    break;
                case 8:
                    if (!check)
                        result.setHomePage(null);
                    break;
                case 9:
                    if (!check)
                        result.setWechat(null);
                    break;
                case 10:
                    if (!check)
                        result.setTwitter(null);
                    break;
                case 11:
                    if (!check)
                        result.setWeibo(null);
                    break;
                case 12:
                    if (!check)
                        result.setFacebook(null);
                    break;
                case 13:
                    if (!check)
                        result.setGoogleAccount(null);
                    break;
            }

        }
        Log.i("??", JSON.toJSONString(result));
        return result;
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getFee":
                Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
                intent.putExtra("wallet", wallet);
                intent.putExtra("type", Constant.CRUPDATE);
                intent.putExtra("chainId", MyWallet.ELA);
                intent.putExtra("ownerPublicKey", ownerPublicKey);
                intent.putExtra("fee", ((CommmonLongEntity) baseEntity).getData());
                intent.putExtra("name", name);
                intent.putExtra("CID", CID);
                intent.putExtra("url", url);
                intent.putExtra("code", code);
                intent.putExtra("transType", 35);

                break;
            case "jwtSave":

                post(RxEnum.SAVECREDENCIALTOWEB.ordinal(), null, null);
                popBackFragment();
                break;
            case "postData":
                showToast(getString(R.string.authoriizesuccess));
                popBackFragment();
                break;
            case "exportxPrivateKey":
                String privateKey = ((CommmonStringEntity) baseEntity).getData();
                String payPasswd = (String) o;
                try {
                    store.initPrivateIdentity(privateKey, payPasswd);
                    generBackJwt(payPasswd);
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
                if ("authorization&bind".equals(type)) {
                    //绑定并授权需要展示手续费
                    new CRSignUpPresenter().getFee(wallet.getWalletId(), MyWallet.ELA, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", type, this);
                } else {
                    Intent intent = new Intent(getActivity(), VertifyPwdActivity.class);
                    intent.putExtra("walletId", wallet.getWalletId());
                    intent.putExtra("type", this.getClass().getSimpleName());
                    startActivity(intent);
                }
                break;
            case R.id.tv_refuse:
                popBackFragment();
                break;


        }
    }

    private void scanJwt(String payPasswd) {
        CallBackJwtEntity callBackJwtEntity = new CallBackJwtEntity();
        callBackJwtEntity.setType("credaccess");
        callBackJwtEntity.setIss(getMyDID().getDidString());
        callBackJwtEntity.setIat(new Date().getTime() / 1000);
        callBackJwtEntity.setExp(new Date().getTime() / 1000 + 5 * 60);
        callBackJwtEntity.setAud(recieveJwtEntity.getIss());
        callBackJwtEntity.setReq(scanResult);
        callBackJwtEntity.setPresentation("");
        String header = jwtParts[0];
        // Base64
        String payload = Base64.encodeToString(JSON.toJSONString(callBackJwtEntity).getBytes(), Base64.URL_SAFE | Base64.NO_WRAP);
        payload = payload.replaceAll("=", "");
        try {
            String signature = getMyDID().getDIDDocument().sign(payPasswd, (header + "." + payload).getBytes());
            new AuthorizationPresenter().postData(callBackUrl, header + "." + payload + "." + signature, this);
        } catch (DIDStoreException e) {
            e.printStackTrace();
        }
    }

    private void authorizationJwt(String payPasswd) {
        /* {
            "alg": "ES256",
                "typ": "JWT"
        }的base64 "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9"*/

        String header = "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9";

        // Base64
        DIDDocument doc = getMyDID().getDIDDocument();
        String pro = JSON.toJSONString(convertCredentialSubjectBean(doc));
        Date exp = getMyDID().getExpires(doc);
        String payload = Base64.encodeToString(getMyDID().getCredentialJson(pro, payPasswd, exp).getBytes(), Base64.URL_SAFE | Base64.NO_WRAP);
        payload = payload.replaceAll("=", "");
        try {
            String signature = doc.sign(payPasswd, (header + "." + payload).getBytes());
            new AuthorizationPresenter().jwtSave(getMyDID().getDidString(), header + "." + payload + "." + signature, this);
            Log.i("??", header + "." + payload + "." + signature);
        } catch (DIDStoreException e) {
            e.printStackTrace();
        }
    }


    /**
     * 生成jwt并给相应的服务器
     *
     * @param payPasswd
     */
    private void generBackJwt(String payPasswd) {
        getMyDID().initDID(payPasswd);
        if ("authorization".equals(type) || "authorization&bind".equals(type)) {
            authorizationJwt(payPasswd);
        } else
            scanJwt(payPasswd);
    }

    private void initDid(String payPasswd) {
        try {
            store = getMyDID().getDidStore();
            if (store.containsPrivateIdentity()) {
                generBackJwt(payPasswd);
            } else {
                //获得私钥用于初始化did
                new CreatMulWalletPresenter().exportxPrivateKey(wallet.getWalletId(), payPasswd, this);
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
            initDid((String) result.getObj());
        }

    }
}
