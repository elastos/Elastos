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

package org.elastos.wallet.ela.ui.vote.ElectoralAffairs;


import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.AppCompatImageView;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.allen.library.SuperButton;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.bean.GetdePositcoinBean;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.activity.TransferActivity;
import org.elastos.wallet.ela.ui.common.bean.CommmonLongEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.find.presenter.VoteFirstPresenter;
import org.elastos.wallet.ela.ui.find.viewdata.RegisteredProducerInfoViewData;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeDotJsonViewData;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeInfoBean;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.SuperNodeListPresenter;
import org.elastos.wallet.ela.ui.vote.UpdateInformation.UpdateInformationFragment;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.svg.GlideApp;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.math.BigDecimal;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 选举管理  getRegisteredProducerInfo
 */
public class ElectoralAffairsFragment extends BaseFragment implements NewBaseViewData, RegisteredProducerInfoViewData {

    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.tv_url)
    TextView tvUrl;
    DialogUtil dialogUtil = new DialogUtil();
    @BindView(R.id.tv_name)
    TextView tvName;
    @BindView(R.id.tv_node)
    TextView tvNode;
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
    @BindView(R.id.ll_intro)
    LinearLayout llIntro;
    @BindView(R.id.ll_tab)
    LinearLayout llTab;
    @BindView(R.id.tv_intro_detail)
    TextView tvIntroDetail;
    @BindView(R.id.ll_infodetail)
    LinearLayout llInfodetail;
    @BindView(R.id.tv_quit)
    TextView tvQuit;
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    ElectoralAffairsPresenter presenter = new ElectoralAffairsPresenter();
    private VoteListBean.DataBean.ResultBean.ProducersBean curentNode;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_electoralaffairs;
    }

    @Override
    protected void initInjector() {

    }


    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.electoral_affairs));
        registReceiver();
    }


    @Override
    protected void setExtraData(Bundle data) {
        curentNode = (VoteListBean.DataBean.ResultBean.ProducersBean) data.getSerializable("curentNode");
        //这里只会有 "Registered", "Canceled"分别代表, 已注册过, 已注销(不知道可不可提取)
        if (curentNode.getState().equals("Canceled")) {
            //已经注销了
            setToobar(toolbar, toolbarTitle, getString(R.string.electoral_affairs));
            ll_xggl.setVisibility(View.GONE);
            ll_tq.setVisibility(View.VISIBLE);
            String nickname = curentNode.getNickname();
            tvQuit.setText(nickname + getString(R.string.hasquit));
            //获取deposit状态
            new VoteFirstPresenter().getRegisteredProducerInfo(wallet.getWalletId(), MyWallet.ELA, this);
        } else {
            //未注销展示选举信息
            onJustRegistered(curentNode);
        }
        super.setExtraData(data);
    }

    @OnClick({R.id.tv_url, R.id.sb_zx, R.id.sb_tq, R.id.sb_up, R.id.ll_info, R.id.ll_intro})
    public void onViewClicked(View view) {
        switch (view.getId()) {
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

            case R.id.sb_zx:
                dialogUtil.showWarmPrompt2(getBaseActivity(), getString(R.string.prompt), new WarmPromptListener() {
                            @Override
                            public void affireBtnClick(View view) {
                                showWarmPromptInput();
                            }
                        }
                );
                break;
            case R.id.sb_tq:
                presenter.createRetrieveDepositTransaction(wallet.getWalletId(), MyWallet.ELA,
                        Arith.mulRemoveZero(available, MyWallet.RATE_S).toPlainString(), this);
                break;

            case R.id.sb_up:
                start(UpdateInformationFragment.class, getArguments());
                break;
        }
    }


    private void showWarmPromptInput() {
        // dialogUtil.showWarmPromptInput(getBaseActivity(), getString(R.string.securitycertificate), getString(R.string.inputWalletPwd), this);

        new CRSignUpPresenter().getFee(wallet.getWalletId(), MyWallet.ELA, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", this);

    }


    String available;

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        Intent intent;
        switch (methodName) {
            case "createRetrieveDepositTransaction":
                intent = new Intent(getActivity(), TransferActivity.class);
                intent.putExtra("wallet", wallet);
                intent.putExtra("type", Constant.WITHDRAWSUPERNODE);
                intent.putExtra("amount", available);
                intent.putExtra("chainId", MyWallet.ELA);
                intent.putExtra("attributes", ((CommmonStringEntity) baseEntity).getData());

                startActivity(intent);
                break;
            case "getDepositcoin":
                GetdePositcoinBean getdePositcoinBean = (GetdePositcoinBean) baseEntity;
                available = getdePositcoinBean.getData().getResult().getAvailable();
                //注销可提取
                sbtq.setVisibility(View.VISIBLE);
                break;
            case "getFee":
                intent = new Intent(getActivity(), VoteTransferActivity.class);
                intent.putExtra("wallet", wallet);
                if (curentNode.getState().equals("Canceled")) {
                    //提取按钮
                    intent.putExtra("type", Constant.WITHDRAWSUPERNODE);
                    intent.putExtra("amount", available);
                    intent.putExtra("transType",12);
                } else {
                    //注销按钮
                    intent.putExtra("type", Constant.UNREGISTERSUPRRNODE);
                    intent.putExtra("transType", 10);
                }
                intent.putExtra("chainId", MyWallet.ELA);
                intent.putExtra("ownerPublicKey", curentNode.getOwnerpublickey());
                intent.putExtra("fee", ((CommmonLongEntity) baseEntity).getData());
                startActivity(intent);
                break;

        }
    }

    private void onJustRegistered(VoteListBean.DataBean.ResultBean.ProducersBean curentNode) {
        tvName.setText(curentNode.getNickname());
        tvAddress.setText(AppUtlis.getLoc(getContext(), curentNode.getLocation() + ""));
        String url = curentNode.getUrl();
        new SuperNodeListPresenter().getUrlJson(url, this, new NodeDotJsonViewData() {
            @Override
            public void onGetNodeDotJsonData(NodeInfoBean t, String url) {
                //获取icon
                if (t == null || t.getOrg() == null || t.getOrg().getBranding() == null || t.getOrg().getBranding().getLogo_256() == null) {
                    return;
                }
                String imgUrl = t.getOrg().getBranding().getLogo_256();
                GlideApp.with(ElectoralAffairsFragment.this).load(imgUrl)
                        .error(R.mipmap.found_vote_initial_circle).circleCrop().into(ivIcon);
                GlideApp.with(ElectoralAffairsFragment.this).load(imgUrl)
                        .error(R.mipmap.found_vote_initial_circle).circleCrop().into(ivIcon1);
                //获取节点简介
                NodeInfoBean.OrgBean.CandidateInfoBean infoBean = t.getOrg().getCandidate_info();
                if (infoBean != null) {
                    String info = new SPUtil(ElectoralAffairsFragment.this.getContext()).getLanguage() == 0 ? infoBean.getZh() : infoBean.getEn();

                    if (!TextUtils.isEmpty(info)) {
                        llTab.setVisibility(View.VISIBLE);
                        tvIntroDetail.setText(info);
                    }
                }

            }
        });
        tvUrl.setText(url);
        tvNode.setText(curentNode.getNodepublickey());
        if (curentNode != null) {
            tvNum.setText(curentNode.getVotes() + getString(R.string.ticket));
            BigDecimal voterateDecimal = new BigDecimal(curentNode.getVoterate());
            if (voterateDecimal.compareTo(new BigDecimal(0.01)) < 0) {
                tv_zb.setText("< 1%");
            } else {
                String voterate = NumberiUtil.numberFormat(Arith.mul(voterateDecimal, 100), 2);
                tv_zb.setText(voterate + "%");
            }
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                    popBackFragment();
                }
            });
        }
    }

    @Override
    public void onGetRegisteredProducerInfo(String data) {

        JSONObject jsonObject = JSON.parseObject(data);
        String status = jsonObject.getString("Status");
        JSONObject info = jsonObject.getJSONObject("Info");
        if (!TextUtils.isEmpty(status) && status.equals("Canceled") && info.getLong("Confirms") >= 2160) {
            //获取押金
            presenter.getDepositcoin(curentNode.getOwnerpublickey(), this);
        }

    }
}
