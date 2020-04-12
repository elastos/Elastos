/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.wallet.ela.ui.did.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonLongEntity;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.entity.DIDInfoEntity;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.Date;

import butterknife.BindView;
import butterknife.OnClick;

public class SocialAccountFragment /*extends BaseFragment implements NewBaseViewData*/ {

   /* @BindView(R.id.tv_title)
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
    @BindView(R.id.tv_tip)
    TextView tvTip;
    private DIDInfoEntity didInfo;
    private CredentialSubjectBean credentialSubjectBean;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_socialaccount;
    }

    @Override
    protected void setExtraData(Bundle data) {
        String type = data.getString("type");
        if (Constant.EDITCREDENTIAL.equals(type)) {
            //编辑did  从凭证信息进入
            onAddPartCredential(data);
            putData();

        } else if (Constant.ADDCREDENTIAL.equals(type)) {
            //新增did  从凭证信息进入
            onAddPartCredential(data);
        } else {
            tvTitle.setText(getString(R.string.addsocialaccount));
            didInfo = data.getParcelable("didInfo");
            credentialSubjectBean = data.getParcelable("credentialSubjectBean");
            if (data.getBoolean("useDraft"))
                putData();
        }
    }


    @Override
    protected void initView(View view) {

        registReceiver();
    }

    private void onAddPartCredential(Bundle data) {
        credentialSubjectBean = data.getParcelable("credentialSubjectBean");
        tvTitle.setText(getString(R.string.editsocialaccount));
        tvTip.setVisibility(View.GONE);
        tvPublic.setVisibility(View.GONE);
        tvKeep.setText(getString(R.string.keep));
        tvKeep.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setData();
                CacheUtil.setCredentialSubjectBean(credentialSubjectBean);
                post(RxEnum.EDITSOCIAL.ordinal(), null, null);
                popTo(CredentialFragment.class, false);
            }
        });
    }

    private void putData() {
        CredentialSubjectBean.Social social = credentialSubjectBean.getSocial();
        if (social != null) {
            etHomepage.setText(social.getHomePage());
            etGoogle.setText(social.getGoogleAccount());
            etMircrosoft.setText(social.getMicrosoftPassport());
            etFacebook.setText(social.getFacebook());
            etTwitter.setText(social.getTwitter());
            etWebo.setText(social.getWeibo());
            etWechat.setText(social.getWechat());
            etAlipay.setText(social.getAlipay());
        }
    }

    private void setData() {
        CredentialSubjectBean.Social social = credentialSubjectBean.getSocial();
        if (social == null) {
            social = new CredentialSubjectBean.Social();
        }
        social.setHomePage(getText(etHomepage));
        social.setGoogleAccount(getText(etGoogle));
        social.setMicrosoftPassport(getText(etMircrosoft));
        social.setFacebook(getText(etFacebook));
        social.setTwitter(getText(etTwitter));
        social.setWeibo(getText(etWebo));
        social.setWechat(getText(etWechat));
        social.setAlipay(getText(etAlipay));
        if (social.isEmpty()) {
            credentialSubjectBean.setInfo(null);
        } else {
            social.setEditTime(new Date().getTime() / 1000);
            credentialSubjectBean.setSocial(social);
        }

    }

    @OnClick({R.id.tv_public, R.id.tv_keep})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_public:

                setData();
                CacheUtil.setCredentialSubjectBean(credentialSubjectBean);
                // keep();
                new CRSignUpPresenter().getFee(didInfo.getWalletId(), MyWallet.IDChain, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", this);
                break;
            case R.id.tv_keep:
                setData();
                CacheUtil.setCredentialSubjectBean(credentialSubjectBean);

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
        didInfo.setIssuanceDate(new Date().getTime() / 1000);
        didInfo.setStatus("Unpublished");
        infoEntities.add(didInfo);
        CacheUtil.setDIDInfoList(infoEntities);
        post(RxEnum.KEEPDRAFT.ordinal(), null, infoEntities);
        showToast(getString(R.string.keepsucess));
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getFee":
                JSONObject json = (JSONObject) JSON.toJSON(didInfo);
                Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
                intent.putExtra("wallet", new RealmUtil().queryUserWallet(didInfo.getWalletId()));
                intent.putExtra("chainId", MyWallet.IDChain);
                intent.putExtra("inputJson", JSON.toJSONString(json));
                intent.putExtra("fee", ((CommmonLongEntity) baseEntity).getData());
                intent.putExtra("type", Constant.DIDSIGNUP);
                intent.putExtra("transType", 10);
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
                    //删除这个草稿?//todo
                    toDIDListFragment();
                }
            });
        }
    }

    @Override
    public boolean onBackPressedSupport() {
        setData();
        return super.onBackPressedSupport();


    }*/
}
