package org.elastos.wallet.ela.ui.did.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonLongEntity;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.did.entity.DIDInfoEntity;
import org.elastos.wallet.ela.ui.main.MainFragment;
import org.elastos.wallet.ela.ui.vote.SuperNodeListFragment;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.Constant;

import java.util.ArrayList;

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
                new CRSignUpPresenter().getFee(wallet.getWalletId(), MyWallet.ELA, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", this);
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
                toDIDListFragment();
                break;


        }
    }

    public void toDIDListFragment() {
        Fragment DIDListFragment = getBaseActivity().getSupportFragmentManager().findFragmentByTag(DIDListFragment.class.getName());
        if (DIDListFragment != null) {
            popTo(DIDListFragment.getClass(),false);
        } else {
            startWithPopTo(new DIDListFragment(), MainFragment.class, false);

        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getFee":
                Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
                intent.putExtra("wallet", wallet);
                intent.putExtra("chainId", MyWallet.IDChain);
                intent.putExtra("inputJson", JSON.toJSONString(credentialSubjectBean));
                intent.putExtra("fee", ((CommmonLongEntity) baseEntity).getData());
                intent.putExtra("type", Constant.DIDSIGNUP);
                startActivity(intent);
                break;
        }

    }
}
