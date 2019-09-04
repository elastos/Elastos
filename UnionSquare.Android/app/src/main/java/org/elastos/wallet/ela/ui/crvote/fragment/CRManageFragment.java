package org.elastos.wallet.ela.ui.crvote.fragment;


import android.os.Bundle;
import android.support.v7.widget.AppCompatImageView;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.allen.library.SuperButton;
import com.blankj.utilcode.util.ToastUtils;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.presenter.PwdPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.crvote.bean.CRDePositcoinBean;
import org.elastos.wallet.ela.ui.crvote.bean.CRMenberInfoBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRManagePresenter;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeDotJsonViewData;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeInfoBean;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.SuperNodeListPresenter;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.GlideApp;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.klog.KLog;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 选举管理  getRegisteredProducerInfo
 */
public class CRManageFragment extends BaseFragment implements WarmPromptListener, CommmonStringWithMethNameViewData, NewBaseViewData {

    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.tv_url)
    TextView tvUrl;
    DialogUtil dialogUtil = new DialogUtil();
    @BindView(R.id.tv_name)
    TextView tvName;
    @BindView(R.id.tv_publickey)
    TextView tvPublickey;
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
    PwdPresenter pwdpresenter = new PwdPresenter();
    String status;
    private String ownerPublicKey;
    private String info;
    private VoteListBean.DataBean.ResultBean.ProducersBean curentNode;

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
    }


    @Override
    protected void setExtraData(Bundle data) {
        status = data.getString("status", "Canceled");
        info = data.getString("info", "");
        curentNode = (VoteListBean.DataBean.ResultBean.ProducersBean) data.getSerializable("curentNode");
        if (curentNode != null) {
            tvNum.setText(curentNode.getVotes() + getString(R.string.ticket));
            tv_zb.setText(NumberiUtil.numberFormat(Double.parseDouble(curentNode.getVoterate()) * 100 + "", 5) + "%");
        }

        ownerPublicKey = data.getString("ownerPublicKey");
        KLog.a(status);
        KLog.a(info);
        //这里只会有 "Registered", "Canceled"分别代表, 已注册过, 已注销(不知道可不可提取)
        if (status.equals("Canceled")) {
            //已经注销了
            setToobar(toolbar, toolbarTitle, getString(R.string.electoral_affairs));
            ll_xggl.setVisibility(View.GONE);
            ll_tq.setVisibility(View.VISIBLE);
            JSONObject jsonObject = JSON.parseObject(info);
            long height = jsonObject.getLong("Confirms");
            presenter = new CRManagePresenter();
            if (height >= 2160) {
                //获取赎回金额
                presenter.getCRDepositcoin(ownerPublicKey, this);
            }
        } else {
            //未注销展示选举信息
            onJustRegistered(info);
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
        dialogUtil.showWarmPromptInput(getBaseActivity(), getString(R.string.securitycertificate), getString(R.string.inputWalletPwd), this);
    }

    String pwd;


    private void onJustRegistered(String data) {

        CRMenberInfoBean bean = JSON.parseObject(data, CRMenberInfoBean.class);
        tvName.setText(bean.getNickName());
        tvAddress.setText(AppUtlis.getLoc(getContext(), bean.getLocation() + ""));
        String url = bean.getURL();
        new SuperNodeListPresenter().getUrlJson(url, this, new NodeDotJsonViewData() {
            @Override
            public void onGetNodeDotJsonData(NodeInfoBean t, String url) {
                //获取icon
                if (t == null || t.getOrg() == null || t.getOrg().getBranding() == null || t.getOrg().getBranding().getLogo_256() == null) {
                    return;
                }
                String imgUrl = t.getOrg().getBranding().getLogo_256();
                GlideApp.with(CRManageFragment.this).load(imgUrl)
                        .error(R.mipmap.found_vote_initial_circle).circleCrop().into(ivIcon);
                //获取节点简介
                NodeInfoBean.OrgBean.CandidateInfoBean infoBean = t.getOrg().getCandidate_info();
                if (infoBean != null) {
                    String info = new SPUtil(CRManageFragment.this.getContext()).getLanguage() == 0 ? infoBean.getZh() : infoBean.getEn();

                    if (!TextUtils.isEmpty(info)) {
                        llTab.setVisibility(View.VISIBLE);
                        tvIntroDetail.setText(info);
                    }
                }

            }
        });
        tvUrl.setText(url);
        tvPublickey.setText(bean.getCROwnerPublicKey());

    }


    //输入密码后
    @Override
    public void affireBtnClick(View view) {
        pwd = ((EditText) view).getText().toString().trim();
        if (TextUtils.isEmpty(pwd)) {
            showToastMessage(getString(R.string.pwdnoempty));
            return;
        }
        presenter.generateUnregisterCRPayload(wallet.getWalletId(), MyWallet.ELA, ownerPublicKey, pwd, this);
    }


    @Override
    public void onGetCommonData(String methodname, String data) {

        switch (methodname) {
            case "signTransaction":
                pwdpresenter.publishTransaction(wallet.getWalletId(), MyWallet.ELA, data, this);
                break;
            case "publishTransaction":
                dialogUtil.dialogDismiss();
                if (status.equals("Canceled")) {
                    ToastUtils.showShort(R.string.deposit_was_retrieved_successfully);
                    sbtq.setVisibility(View.GONE);
                } else {
                    ll_xggl.setVisibility(View.GONE);
                    ll_tq.setVisibility(View.VISIBLE);
                    ToastUtils.showShort(getString(R.string._72));
                }
                break;

        }
    }


    String available;


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

        switch (methodName) {

            case "generateUnregisterCRPayload":

                if (status.equals("Canceled")) {
                    //提取按钮
                    presenter.createRetrieveCRDepositTransaction(wallet.getWalletId(), MyWallet.ELA,
                            Arith.sub(Arith.mul(available, MyWallet.RATE_S), "10000").toPlainString(), "", this);
                } else {
                    //注销按钮
                    presenter.createUnregisterCRTransaction(wallet.getWalletId(), MyWallet.ELA, "", ((CommmonStringEntity) baseEntity).getData(), "", false, this);
                }

                break;
            case "createRetrieveCRDepositTransaction":
            case "createUnregisterCRTransaction":
                pwdpresenter.signTransaction(wallet.getWalletId(), MyWallet.ELA, ((CommmonStringEntity) baseEntity).getData(), pwd, this);
                break;

            case "getCRDepositcoin":
                CRDePositcoinBean getdePositcoinBean = (CRDePositcoinBean) baseEntity;
                available = getdePositcoinBean.getData().getResult().getAvailable();
                //注销可提取
                sbtq.setVisibility(View.VISIBLE);
                break;


        }
    }
}
