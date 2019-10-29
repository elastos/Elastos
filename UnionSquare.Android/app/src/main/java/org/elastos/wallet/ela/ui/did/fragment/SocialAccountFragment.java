package org.elastos.wallet.ela.ui.did.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonLongEntity;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.did.entity.DIDInfoEntity;
import org.elastos.wallet.ela.ui.main.MainFragment;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.Date;

import butterknife.BindView;
import butterknife.OnClick;

public class SocialAccountFragment extends BaseFragment implements NewBaseViewData {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.et_homepage)
    EditText etHomepage;
    @BindView(R.id.et_google)
    EditText etGoogle;
    @BindView(R.id.et_mircrosoft)
    EditText etMircrosoft;
    @BindView(R.id.et_facebook)
    EditText etFacebook;
    @BindView(R.id.et_twitter)
    EditText etTwitter;
    @BindView(R.id.et_webo)
    EditText etWebo;
    @BindView(R.id.et_wechat)
    EditText etWechat;
    @BindView(R.id.et_alipay)
    EditText etAlipay;
    @BindView(R.id.tv_public)
    TextView tvPublic;
    @BindView(R.id.tv_keep)
    TextView tvKeep;
    Wallet wallet;
    private DIDInfoEntity didInfo;
    private DIDInfoEntity.CredentialSubjectBean credentialSubjectBean;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_socialaccount;
    }

    @Override
    protected void setExtraData(Bundle data) {
        didInfo = data.getParcelable("didInfo");
        credentialSubjectBean = didInfo.getCredentialSubject();
        wallet = data.getParcelable("wallet");
        if (data.getBoolean("useDraft"))
            putData();
    }


    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.addsocialaccount));
        registReceiver();
    }

    private void putData() {
        etHomepage.setText(credentialSubjectBean.getHomePage());
        etGoogle.setText(credentialSubjectBean.getGoogleAccount());
        etMircrosoft.setText(credentialSubjectBean.getMicrosoftPassport());
        etFacebook.setText(credentialSubjectBean.getFacebook());
        etTwitter.setText(credentialSubjectBean.getTwitter());
        etWebo.setText(credentialSubjectBean.getWeibo());
        etWechat.setText(credentialSubjectBean.getWechat());
        etAlipay.setText(credentialSubjectBean.getAlipay());
    }

    private void setData() {
        credentialSubjectBean.setHomePage(getText(etHomepage));
        credentialSubjectBean.setGoogleAccount(getText(etGoogle));
        credentialSubjectBean.setMicrosoftPassport(getText(etMircrosoft));
        credentialSubjectBean.setFacebook(getText(etFacebook));
        credentialSubjectBean.setTwitter(getText(etTwitter));
        credentialSubjectBean.setWeibo(getText(etWebo));
        credentialSubjectBean.setWechat(getText(etWechat));
        credentialSubjectBean.setAlipay(getText(etAlipay));


    }

    @OnClick({R.id.tv_public, R.id.tv_keep})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_public:
                setData();
                new CRSignUpPresenter().getFee(wallet.getWalletId(), MyWallet.ELA, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", this);
                break;
            case R.id.tv_keep:
                setData();
                keep();
                toDIDListFragment();
                break;


        }
    }

    private void keep() {
        ArrayList<DIDInfoEntity> infoEntities = CacheUtil.getDIDInfoList();
        if (infoEntities.contains(didInfo)) {
            infoEntities.remove(didInfo);
        }
        didInfo.setIssuanceDate(new Date().getTime()/1000+"");
        didInfo.setStatus("Unpublished");
        infoEntities.add(didInfo);
        CacheUtil.setDIDInfoList(infoEntities);
        post(RxEnum.KEEPDRAFT.ordinal(), null, null);
        showToast(getString(R.string.keepsucess));
    }

    public void toDIDListFragment() {
        Fragment DIDListFragment = getBaseActivity().getSupportFragmentManager().findFragmentByTag(DIDListFragment.class.getName());
        if (DIDListFragment != null) {
            popTo(DIDListFragment.getClass(), false);
        } else {
            startWithPopTo(new DIDListFragment(), MainFragment.class, false);

        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getFee":
                JSONObject json = (JSONObject) JSON.toJSON(didInfo);
                json.remove("credentialSubject");
                Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
                intent.putExtra("wallet", wallet);
                intent.putExtra("chainId", MyWallet.ELA);
                intent.putExtra("inputJson", JSON.toJSONString(json));
                intent.putExtra("fee", ((CommmonLongEntity) baseEntity).getData());
                intent.putExtra("type", Constant.DIDSIGNUP);
                startActivity(intent);
                break;
        }

    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                    toDIDListFragment();
                }
            });
        }
    }
}
