package org.elastos.wallet.ela.ui.crvote.fragment;


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
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonLongEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.crvote.bean.CRDePositcoinBean;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.bean.CRMenberInfoBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRManagePresenter;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeDotJsonViewData;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeInfoBean;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.SuperNodeListPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.GlideApp;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

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

    @BindView(R.id.sb_zx)
    SuperButton sbZx;
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    CRManagePresenter presenter;
    String status;
    private String info;
    private String ownerPublicKey;
    private String did;


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
        registReceiver();
    }


    @Override
    protected void setExtraData(Bundle data) {
        status = data.getString("status", "Canceled");
        did = data.getString("did", "");
        info = data.getString("info", "");
        CRListBean.DataBean.ResultBean.CrcandidatesinfoBean curentNode = (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean) data.getSerializable("curentNode");

        presenter = new CRManagePresenter();

        //这里只会有 "Registered", "Canceled"分别代表, 已注册过, 已注销(不知道可不可提取)
        if (status.equals("Canceled")) {
            //已经注销了
            setToobar(toolbar, toolbarTitle, getString(R.string.electoral_affairs));
            ll_xggl.setVisibility(View.GONE);
            ll_tq.setVisibility(View.VISIBLE);
            JSONObject jsonObject = JSON.parseObject(info);
            long height = jsonObject.getLong("Confirms");
            if (height >= 2160) {
                //获取赎回金额
                new CRSignUpPresenter().getCROwnerPublicKey(wallet.getWalletId(), MyWallet.ELA, this);
            }
        } else {
            //Registered 未注销展示选举信息
            onJustRegistered(info, curentNode);
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
                dialogUtil.showWarmPrompt2(getBaseActivity(), getString(R.string.quitcrornot), new WarmPromptListener() {
                            @Override
                            public void affireBtnClick(View view) {
                                showWarmPromptInput();
                            }
                        }
                );
                break;
            case R.id.sb_tq:
                showWarmPromptInput();
                break;

            case R.id.sb_up:
                Bundle bundle = new Bundle();
                bundle.putString("info", info);
                start(UpdateCRInformationFragment.class, bundle);
                break;
        }
    }


    private void showWarmPromptInput() {
        new CRSignUpPresenter().getFee(wallet.getWalletId(), MyWallet.ELA, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", this);


    }

    String pwd;


    private void onJustRegistered(String data, CRListBean.DataBean.ResultBean.CrcandidatesinfoBean curentNode) {

        CRMenberInfoBean bean = JSON.parseObject(data, CRMenberInfoBean.class);
        tvName.setText(bean.getNickName());
        tvAddress.setText(AppUtlis.getLoc(getContext(), bean.getLocation() + ""));
        String url = bean.getURL();
        new SuperNodeListPresenter().getCRUrlJson(url, this, new NodeDotJsonViewData() {
            @Override
            public void onGetNodeDotJsonData(NodeInfoBean t, String url) {
                //获取icon

                try {
                    String imgUrl = t.getOrg().getBranding().getLogo_256();
                    GlideApp.with(CRManageFragment.this).load(imgUrl)
                            .error(R.mipmap.found_vote_initial_circle).circleCrop().into(ivIcon);
                } catch (Exception e) {
                }
                try {
                    //获取节点简介
                    NodeInfoBean.OrgBean.CandidateInfoBean infoBean = t.getOrg().getCandidate_info();

                    String info = new SPUtil(CRManageFragment.this.getContext()).getLanguage() == 0 ? infoBean.getZh() : infoBean.getEn();
                    if (!TextUtils.isEmpty(info)) {
                        llTab.setVisibility(View.VISIBLE);
                        tvIntroDetail.setText(info);
                    }

                } catch (Exception e) {
                }
            }
        });
        tvUrl.setText(url);
        tvDid.setText(bean.getCROwnerDID());
        if (curentNode != null) {
            tvNum.setText(curentNode.getVotes() + getString(R.string.ticket));
            tv_zb.setText(curentNode.getVoterate() + "%");
        }
        ownerPublicKey = bean.getCROwnerPublicKey();
    }


    String available;


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

        switch (methodName) {
            case "getFee":
                Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
                intent.putExtra("wallet", wallet);
                if (status.equals("Canceled")) {
                    //提取按钮
                    intent.putExtra("type", Constant.WITHDRAWCR);
                    intent.putExtra("amount", available);
                    intent.putExtra("transType", 36);
                } else {
                    //注销按钮
                    intent.putExtra("type", Constant.UNREGISTERCR);
                    intent.putExtra("transType", 34);
                }
                intent.putExtra("chainId", MyWallet.ELA);
                intent.putExtra("did", did);
                intent.putExtra("fee", ((CommmonLongEntity) baseEntity).getData());
                startActivity(intent);
                break;


            case "getCRDepositcoin":
                CRDePositcoinBean getdePositcoinBean = (CRDePositcoinBean) baseEntity;
                available = getdePositcoinBean.getData().getResult().getAvailable();
                //注销可提取
                sbtq.setVisibility(View.VISIBLE);
                break;

            //获取钱包owner公钥
            case "getCROwnerPublicKey":
                ownerPublicKey = ((CommmonStringEntity) baseEntity).getData();
                //getdepositcoin();//获取赎回金额
                presenter.getCRDepositcoin(did, this);
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
                    popBackFragment();
                }
            });
        }
    }
}
