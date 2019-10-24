package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.did.entity.DIDInfoEntity;
import org.elastos.wallet.ela.utils.CacheUtil;

import java.util.ArrayList;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class SocialAccountFragment extends BaseFragment {

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
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.addsocialaccount));

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
                break;
            case R.id.tv_keep:
                setData();
                ArrayList<DIDInfoEntity> infoEntities = CacheUtil.getDIDInfoList();
                if (infoEntities.contains(didInfo)) {
                    infoEntities.remove(didInfo);
                }
                infoEntities.add(didInfo);
                CacheUtil.setDIDInfoList(infoEntities);
                showToast(getString(R.string.keepsucess));
                toMainFragment();
                break;


        }
    }


}
