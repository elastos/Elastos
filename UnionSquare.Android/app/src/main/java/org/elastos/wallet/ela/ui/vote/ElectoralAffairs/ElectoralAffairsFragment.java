package org.elastos.wallet.ela.ui.vote.ElectoralAffairs;


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
import org.elastos.wallet.ela.bean.GetdePositcoinBean;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.net.ApiServer;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.ui.Assets.presenter.PwdPresenter;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeDotJsonViewData;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeInfoBean;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.SuperNodeListPresenter;
import org.elastos.wallet.ela.ui.vote.UpdateInformation.UpdateInformationFragment;
import org.elastos.wallet.ela.ui.vote.bean.ElectoralAffairsBean;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.GlideApp;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxSchedulers;
import org.elastos.wallet.ela.utils.klog.KLog;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;

import java.util.HashMap;
import java.util.Map;

import butterknife.BindView;
import butterknife.OnClick;
import io.reactivex.functions.Consumer;

/**
 * 选举管理  getRegisteredProducerInfo
 */
public class ElectoralAffairsFragment extends BaseFragment implements WarmPromptListener, CommmonStringWithMethNameViewData, VotelistViewData {

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
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    ElectoralAffairsPresenter presenter = new ElectoralAffairsPresenter();
    PwdPresenter pwdpresenter = new PwdPresenter();
    String status;
    private String ownerPublicKey;
    private String info;

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
    }


    @Override
    protected void setExtraData(Bundle data) {
        status = data.getString("status", "Canceled");
        info = data.getString("info", "");
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
            if (height >= 2160) {
                //注销可提取
                sbtq.setVisibility(View.VISIBLE);
                //获取交易所需公钥
                presenter.getPublicKeyForVote(wallet.getWalletId(), MyWallet.ELA, this);
            }
        } else {
            //未注销展示选举信息
            onJustRegistered(info);
        }
        super.setExtraData(data);
    }

    @OnClick({R.id.tv_url, R.id.sb_keystore, R.id.sb_zx, R.id.sb_tq, R.id.sb_up})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_url:
                //复制
                ClipboardUtil.copyClipboar(getBaseActivity(), tvUrl.getText().toString());
                break;
            case R.id.sb_keystore:
                // start(ImportNodeKeystoreFragment.class);
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
                start(UpdateInformationFragment.class, bundle);
                break;
        }
    }


    private void showWarmPromptInput() {
        dialogUtil.showWarmPromptInput(getBaseActivity(), getString(R.string.securitycertificate), getString(R.string.inputWalletPwd), this);
    }

    String pwd, data;


    private void onJustRegistered(String data) {
        this.data = data;
        ElectoralAffairsBean bean = JSON.parseObject(data, ElectoralAffairsBean.class);
        tvName.setText(bean.getNickName());
        tvAddress.setText(AppUtlis.getLoc(getContext(), bean.getLocation() + ""));
        String url = bean.getURL();
        new SuperNodeListPresenter().getUrlJson(url, getContext(), new NodeDotJsonViewData() {
            @Override
            public void onGetNodeDotJsonData(NodeInfoBean t, String url) {
                if (t == null || t.getOrg() == null || t.getOrg().getBranding() == null) {
                    return;
                }
                String imgUrl = t.getOrg().getBranding().getLogo_256();
                GlideApp.with(ElectoralAffairsFragment.this).load(imgUrl)
                        .error(R.mipmap.found_vote_initial_circle).into(ivIcon);
            }
        });
        tvUrl.setText(url);
        tvNode.setText(bean.getNodePublicKey());
        ownerPublicKey = bean.getOwnerPublicKey();
        // tv_zb.setText(bean.getInfo().);
        // HttpList();
        new VoteListPresenter().votelistbean("1", this);
    }


    //输入密码后
    @Override
    public void affireBtnClick(View view) {
        pwd = ((EditText) view).getText().toString().trim();
        if (TextUtils.isEmpty(pwd)) {
            showToastMessage(getString(R.string.pwdnoempty));
            return;
        }
        presenter.generateCancelProducerPayload(wallet.getWalletId(), MyWallet.ELA, ownerPublicKey, pwd, this);
    }


    @Override
    public void onGetCommonData(String methodname, String data) {

        switch (methodname) {

            case "generateCancelProducerPayload":
                KLog.a(data);
                if (status.equals("Canceled")) {
                    //提取按钮
                    presenter.createRetrieveDepositTransaction(wallet.getWalletId(), MyWallet.ELA,
                            Arith.sub(Arith.mul(available, MyWallet.RATE_S),"10000").toPlainString(), "", this);
                } else {
                    //注销按钮
                    presenter.createCancelProducerTransaction(wallet.getWalletId(), MyWallet.ELA, "", data, "", false, this);
                }

                break;
            case "createRetrieveDepositTransaction":
            case "createCancelProducerTransaction":
                pwdpresenter.signTransaction(wallet.getWalletId(), MyWallet.ELA, data, pwd, this);
                break;
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

            //获取钱包owner公钥
            case "getPublicKeyForVote":
                ownerPublicKey = data;
                getdepositcoin();//获取赎回金额
                break;

        }
    }


    String available;

    public void getdepositcoin() {
        Map<String, String> map = new HashMap();
        map.put("ownerpublickey", ownerPublicKey);
        RetrofitManager.create(ApiServer.class, getContext())
                .getdepositcoin(map)
                .compose(RxSchedulers.<GetdePositcoinBean>applySchedulers())
                .subscribe(new Consumer<GetdePositcoinBean>() {
                    @Override
                    public void accept(GetdePositcoinBean dataResponse) {
                        available = dataResponse.getData().getResult().getAvailable();
                    }
                }, new Consumer<Throwable>() {
                    @Override
                    public void accept(Throwable throwable) {

                    }
                });
    }

    @Override
    public void onGetVoteList(VoteListBean dataResponse) {


        if (dataResponse.getData().getResult().getProducers() != null) {

            for (int i = 0; i < dataResponse.getData().getResult().getProducers().size(); i++) {

                if (dataResponse.getData().getResult().getProducers().get(i).getOwnerpublickey().equals(ownerPublicKey)) {
                    tvNum.setText(dataResponse.getData().getResult().getProducers().get(i).getVotes() + getString(R.string.ticket));
                    if (dataResponse.getData().getResult().getProducers().get(i).getVoterate() != null) {
                        tv_zb.setText(NumberiUtil.numberFormat(Double.parseDouble(dataResponse.getData().getResult().getProducers().get(i).getVoterate()) * 100 + "", 5) + "%");
                    }
                }

            }
        }


    }

}
