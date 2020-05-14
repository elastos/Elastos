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

package org.elastos.wallet.ela.ui.crvote.fragment;


import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.AppCompatImageView;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.text.method.ScrollingMovementMethod;
import android.util.Base64;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.allen.library.SuperButton;

import org.elastos.did.DIDDocument;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.activity.TransferActivity;
import org.elastos.wallet.ela.ui.Assets.presenter.WalletManagePresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonLongEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonObjEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.crvote.bean.CRDePositcoinBean;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.bean.CrStatusBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRManagePresenter;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.entity.GetJwtRespondBean;
import org.elastos.wallet.ela.ui.did.fragment.AuthorizationFragment;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.JwtUtils;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.elastos.wallet.ela.utils.svg.GlideApp;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.Date;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 选举管理  getRegisteredProducerInfo
 */
public class CRManageFragment extends BaseFragment implements NewBaseViewData {

    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.tv_url)
    TextView tvUrl;
    DialogUtil dialogUtil = new DialogUtil();
    @BindView(R.id.tv_name)
    TextView tvName;
    @BindView(R.id.tv_did)
    TextView tvDid;
    @BindView(R.id.tv_num)
    TextView tvNum;
    @BindView(R.id.tv_address)
    TextView tvAddress;
    @BindView(R.id.ll_xggl)
    LinearLayout ll_xggl;
    @BindView(R.id.ll_tq)
    LinearLayout ll_tq;
    @BindView(R.id.sb_tq)
    SuperButton sbtq;
    @BindView(R.id.tv_zb)
    TextView tv_zb;
    @BindView(R.id.sb_up)
    SuperButton sb_up;
    @BindView(R.id.iv_icon)
    AppCompatImageView ivIcon;
    @BindView(R.id.iv_icon1)
    AppCompatImageView ivIcon1;
    @BindView(R.id.line_info)
    View lineInfo;
    @BindView(R.id.tv_info)
    TextView tvInfo;
    @BindView(R.id.ll_info)
    LinearLayout llInfo;
    @BindView(R.id.line_intro)
    View lineIntro;
    @BindView(R.id.tv_intro)
    TextView tvIntro;
    @BindView(R.id.tv_quit)
    TextView tvQuit;
    @BindView(R.id.ll_intro)
    LinearLayout llIntro;
    @BindView(R.id.ll_tab)
    LinearLayout llTab;
    @BindView(R.id.tv_intro_detail)
    TextView tvIntroDetail;
    @BindView(R.id.ll_infodetail)
    LinearLayout llInfodetail;
    @BindView(R.id.iv_detail)
    ImageView ivDetail;
    @BindView(R.id.tv_title_right)
    TextView tvTitleRight;
    private Wallet wallet;
    CRManagePresenter presenter;
    private String CID, DID;
    private String ownerPublicKey;
    private CredentialSubjectBean credentialSubjectBean;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_cr_manage;
    }

    @Override
    protected void initInjector() {

    }


    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.electoral_affairs));
        tvTitleRight.setText(getString(R.string.quitcr));
        tvIntroDetail.setMovementMethod(ScrollingMovementMethod.getInstance());
        registReceiver();
    }


    @Override
    protected void setExtraData(Bundle data) {

        wallet = data.getParcelable("wallet");
        CrStatusBean crStatusBean = data.getParcelable("crStatusBean");
        CrStatusBean.InfoBean info = crStatusBean.getInfo();
        ownerPublicKey = info.getCROwnerPublicKey();
        CID = info.getCID();
        DID = info.getDID();
        CRListBean.DataBean.ResultBean.CrcandidatesinfoBean curentNode = (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean) data.getSerializable("curentNode");
        //curentNode只是用来展示信息
        if (curentNode == null) {
            return;
        }

        presenter = new CRManagePresenter();
        //这里只会有 "Registered", "Canceled"分别代表, 已注册过, 已注销(不知道可不可提取)
        if (crStatusBean.getStatus().equals("Canceled")) {
            //已经注销了
            setToobar(toolbar, toolbarTitle, getString(R.string.electoral_affairs));
            ll_xggl.setVisibility(View.GONE);
            ll_tq.setVisibility(View.VISIBLE);
            tvQuit.setText(curentNode.getNickname() + getString(R.string.hasquit));
            long height = info.getConfirms();
            if (height >= 2160) {
                //获取赎回金额
                presenter.getCRDepositcoin(CID, this);
            }
        } else {
            //Registered 未注销展示选举信息
            onJustRegistered(info, curentNode);
        }
        super.setExtraData(data);
    }

    @OnClick({R.id.tv_url, R.id.sb_zx, R.id.sb_tq, R.id.sb_up, R.id.ll_info, R.id.ll_intro, R.id.tv_title_right, R.id.iv_detail, R.id.tv_did})
    public void onViewClicked(View view) {
        Bundle bundle;
        switch (view.getId()) {
            case R.id.tv_did:
                if (!TextUtils.isEmpty(DID))
                    ClipboardUtil.copyClipboar(getBaseActivity(), tvDid.getText().toString());
                break;
            case R.id.iv_detail:
                bundle = new Bundle();
                bundle.putParcelable("credentialSubjectBean", credentialSubjectBean);
                start(CredentialInfoFragemnt.class, bundle);
                break;
            case R.id.sb_zx:
                //更新did信息
                if (!TextUtils.isEmpty(DID)) {
                    //已经绑定did
                    //直接授权页更新凭证到中心化服务器
                    bundle = new Bundle();
                    bundle.putString("type", "authorization");
                    bundle.putParcelable("wallet", wallet);
                    start(AuthorizationFragment.class, bundle);
                    return;
                }
                //先绑定did  再更新到服务器
                new WalletManagePresenter().DIDResolveWithTip(wallet.getDid(), this, "1");
                break;
            case R.id.ll_info:
                lineInfo.setVisibility(View.VISIBLE);
                lineIntro.setVisibility(View.GONE);
                tvInfo.setTextColor(getResources().getColor(R.color.whiter));
                tvIntro.setTextColor(getResources().getColor(R.color.whiter50));
                llInfodetail.setVisibility(View.VISIBLE);
                tvIntroDetail.setVisibility(View.GONE);
                break;
            case R.id.ll_intro:
                lineInfo.setVisibility(View.GONE);
                lineIntro.setVisibility(View.VISIBLE);
                tvInfo.setTextColor(getResources().getColor(R.color.whiter50));
                tvIntro.setTextColor(getResources().getColor(R.color.whiter));
                llInfodetail.setVisibility(View.GONE);
                tvIntroDetail.setVisibility(View.VISIBLE);
                break;
            case R.id.tv_url:
                //复制
                ClipboardUtil.copyClipboar(getBaseActivity(), tvUrl.getText().toString());
                break;

            case R.id.tv_title_right:
                dialogUtil.showWarmPrompt2(getBaseActivity(), getString(R.string.quitcrornot), new WarmPromptListener() {
                            @Override
                            public void affireBtnClick(View view) {
                                showWarmPromptInput(Constant.UNREGISTERCR);
                            }
                        }
                );
                break;
            case R.id.sb_tq:
                presenter.createRetrieveCRDepositTransaction(wallet.getWalletId(), MyWallet.ELA, ownerPublicKey,
                        Arith.mulRemoveZero(available, MyWallet.RATE_S).toPlainString(), "", this);

                break;

            case R.id.sb_up:
                if (!TextUtils.isEmpty(DID)) {
                    start(UpdateCRInformationFragment.class, getArguments());
                    return;
                }
                new WalletManagePresenter().DIDResolveWithTip(wallet.getDid(), this, "2");
                break;
        }
    }


    private void showWarmPromptInput(String type) {
        new CRSignUpPresenter().getFee(wallet.getWalletId(), MyWallet.ELA, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", type, this);


    }


    private void onJustRegistered(CrStatusBean.InfoBean bean, CRListBean.DataBean.ResultBean.CrcandidatesinfoBean curentNode) {
        tvTitleRight.setVisibility(View.VISIBLE);
        tvName.setText(bean.getNickName());
        tvAddress.setText(AppUtlis.getLoc(getContext(), bean.getLocation() + ""));
        String url = bean.getURL();
        tvUrl.setText(url);
        if (!TextUtils.isEmpty(DID)) {
            DID = "did:elastos:" + DID;
            tvDid.setText(DID);
            //从服务器获得凭证信息
            new CRManagePresenter().jwtGet(DID, this);
        } else {
            tvDid.setText(getString(R.string.unactive));
            tvDid.setCompoundDrawables(null, null, null, null);
        }
        if (curentNode != null) {
            tvNum.setText(curentNode.getVotes().split("\\.")[0] + " " + getString(R.string.ticket));
            tv_zb.setText(curentNode.getVoterate() + "%");
        }
    }


    String available;


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        Intent intent;
        switch (methodName) {
            case "jwtGet":
                GetJwtRespondBean getJwtRespondBean = (GetJwtRespondBean) baseEntity;
                String jwt = getJwtRespondBean.getData().getJwt();
                if (!TextUtils.isEmpty(jwt)) {
                    String payload = JwtUtils.getJwtPayload(jwt);
                    String pro = getMyDID().getCredentialProFromJson(payload);
                    credentialSubjectBean = JSON.parseObject(pro, CredentialSubjectBean.class);
                    if (credentialSubjectBean == null || credentialSubjectBean.whetherEmpty()) {
                        return;
                    }
                    ivDetail.setVisibility(View.VISIBLE);
                    GlideApp.with(CRManageFragment.this).load(credentialSubjectBean.getAvatar())
                            .error(R.mipmap.found_vote_initial_circle).circleCrop().into(ivIcon);
                    GlideApp.with(CRManageFragment.this).load(credentialSubjectBean.getAvatar())
                            .error(R.mipmap.found_vote_initial_circle).circleCrop().into(ivIcon1);
                    if (!TextUtils.isEmpty(credentialSubjectBean.getIntroduction())) {
                        llTab.setVisibility(View.VISIBLE);
                        tvIntroDetail.setText(credentialSubjectBean.getIntroduction());
                    }
                }
                break;
            case "createRetrieveCRDepositTransaction":
                intent = new Intent(getActivity(), TransferActivity.class);
                intent.putExtra("wallet", wallet);
                intent.putExtra("type", Constant.WITHDRAWCR);
                intent.putExtra("amount", available);
                intent.putExtra("chainId", MyWallet.ELA);
                intent.putExtra("transType", 36);
                intent.putExtra("attributes", ((CommmonStringEntity) baseEntity).getData());
                startActivity(intent);
                break;
            case "getFee":
                String type = (String) o;
                intent = new Intent(getActivity(), VoteTransferActivity.class);
                intent.putExtra("wallet", wallet);
                intent.putExtra("type", type);
                intent.putExtra("chainId", MyWallet.ELA);
                intent.putExtra("CID", CID);
                intent.putExtra("fee", ((CommmonLongEntity) baseEntity).getData());
                //注销按钮
                intent.putExtra("transType", 34);
                startActivity(intent);

                break;


            case "getCRDepositcoin":
                CRDePositcoinBean getdePositcoinBean = (CRDePositcoinBean) baseEntity;
                available = getdePositcoinBean.getData().getResult().getAvailable();
                //注销可提取
                sbtq.setVisibility(View.VISIBLE);
                break;
            case "DIDResolveWithTip":
                String resolveType = (String) o;
                DIDDocument didDocument = (DIDDocument) ((CommmonObjEntity) baseEntity).getData();
                if (didDocument == null) {
                    showToast(getString(R.string.notcreatedid));
                    return;
                }
                if (getMyDID().getExpires(didDocument).before(new Date())) {
                    //did过期
                    showToast(getString(R.string.didoutofdate));
                    return;

                }
                //已经注册did  且未绑定
                if ("1".equals(resolveType)) {
                    //在授权页面绑定并授权
                    Bundle bundle = getArguments();
                    bundle.putString("type", "authorization&bind");
                    start(AuthorizationFragment.class, bundle);
                } else {
                    //在升级页面绑定并更新(更新包含绑定)
                    start(UpdateCRInformationFragment.class, getArguments());
                }

                break;

        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.TRANSFERSUCESS.ordinal() && "34".equals(result.getName())) {
            //退出参选成功
            new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                    popBackFragment();
                }
            });


        }
        if (integer == RxEnum.SAVECREDENCIALTOWEB.ordinal()) {
            if (!TextUtils.isEmpty(DID)) {
                showToast(getString(R.string.update_successful));
                new CRManagePresenter().jwtGet(DID, this);
            }
        }
    }


}
