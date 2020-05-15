package org.elastos.wallet.ela.ui.proposal.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Base64;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.google.gson.Gson;

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
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecieveProposalJwtEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.PwdPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.mulwallet.CreatMulWalletPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringWithiMethNameEntity;
import org.elastos.wallet.ela.ui.did.presenter.AuthorizationPresenter;
import org.elastos.wallet.ela.ui.proposal.adapter.SuggetMoneyRecAdapetr;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalCallBackEntity;
import org.elastos.wallet.ela.ui.proposal.bean.SuggestBean;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalPresenter;
import org.elastos.wallet.ela.ui.proposal.presenter.bean.ProposalCRCouncialMenberDigestPayLoad;
import org.elastos.wallet.ela.ui.proposal.presenter.bean.ProposalOwnerDigestPayLoad;
import org.elastos.wallet.ela.ui.vote.activity.VertifyPwdActivity;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.JwtUtils;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.json.JSONException;
import org.json.JSONObject;

import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class SuggestionsInfoFragment extends BaseFragment implements NewBaseViewData {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.tv_title_right)
    TextView tvTitleRight;
    @BindView(R.id.tv_suggest_tile)
    TextView tvSuggestTile;
    @BindView(R.id.tv_num)
    TextView tvNum;
    @BindView(R.id.tv_time)
    TextView tvTime;
    @BindView(R.id.tv_people)
    TextView tvPeople;
    @BindView(R.id.tv_type)
    TextView tvType;
    @BindView(R.id.tv_hash)
    TextView tvHash;
    @BindView(R.id.tv_did)
    TextView tvDid;
    @BindView(R.id.tv_account)
    TextView tvAccount;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.tv_abs)
    TextView tvAbs;
    @BindView(R.id.tv_sign)
    TextView tvSign;
    @BindView(R.id.tv_tag_money)
    TextView tvTagMoney;
    @BindView(R.id.tv_count)
    TextView tvCount;
    private Wallet wallet;
    private String scanResult;
    private RecieveProposalJwtEntity entity;
    private String payPasswd;
    private DIDStore store;
    String command;
    ProposalCRCouncialMenberDigestPayLoad targetEntity;
    private ProposalPresenter presenter;

    @Override
    protected void setExtraData(Bundle data) {
        wallet = data.getParcelable("wallet");
        scanResult = data.getString("scanResult");
        String result = scanResult.replace("elastos://crproposal/", "");
        String payload = JwtUtils.getJwtPayload(result);
        //command.equals("createsuggestion")||command.equals("createproposal")
        command = data.getString("command");
        entity = JSON.parseObject(payload, RecieveProposalJwtEntity.class);
    }

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_proposal_suggest;
    }

    @Override
    protected void initView(View view) {
        setQrData();
        presenter = new ProposalPresenter();
        presenter.getSuggestion(entity.getSid(), this);
        if (command.equals("createsuggestion")) {
            tvTitle.setText(R.string.suggest);
        } else {
            tvTitle.setText(R.string.putproposal);
        }
        registReceiver();

    }

    private void setQrData() {
        RecieveProposalJwtEntity.DataBean data = entity.getData();
        String originType = data.getProposaltype().toLowerCase();
        if ("Normal".toLowerCase().equals(originType)) {
            tvType.setText(R.string.common);
        } else if ("ELIP".toLowerCase().equals(originType)) {
            tvType.setText("ELIP");
        }
        tvHash.setText(data.getDrafthash());
        tvAccount.setText(data.getRecipient());
    }

    @OnClick({R.id.tv_sign})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sign:
                //签名
                if ("createsuggestion".equals(command)) {
                    toVertifyPwdActivity();

                } else if ("createproposal".equals(command)) {
                    showFeePage();
                }

                break;


        }
    }

    private void showFeePage() {
        //发送提案交易
        Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
        intent.putExtra("wallet", wallet);
        intent.putExtra("chainId", MyWallet.ELA);
        intent.putExtra("fee", 10000L);
        intent.putExtra("type", Constant.PROPOSALINPUT);
        intent.putExtra("transType", 37);
        startActivity(intent);
    }

    private void ConverProposalOwnerDigestPayLoad(RecieveProposalJwtEntity entity) {
        RecieveProposalJwtEntity.DataBean data = entity.getData();
        //ProposalOwnerDigestPayLoad targetEntity = new ProposalOwnerDigestPayLoad();
        String originType = data.getProposaltype().toLowerCase();
        if ("Normal".toLowerCase().equals(originType)) {
            targetEntity.setType(0x0000);
        } else if ("ELIP".toLowerCase().equals(originType)) {
            targetEntity.setType(0x0100);
        }

        targetEntity.setCategoryData(data.getCategorydata());
        targetEntity.setOwnerPublicKey(data.getOwnerpublickey());
        targetEntity.setDraftHash(data.getDrafthash());
        List<ProposalOwnerDigestPayLoad.BudgetsBean> bugetList = new ArrayList<>();
        for (RecieveProposalJwtEntity.DataBean.BudgetsBean orginBuget : data.getBudgets()) {
            ProposalOwnerDigestPayLoad.BudgetsBean targetBudget = new ProposalOwnerDigestPayLoad.BudgetsBean();
            switch (orginBuget.getType().toLowerCase()) {
                case "imprest":
                    targetBudget.setType(0);
                    break;
                case "normalpayment":
                    targetBudget.setType(1);
                    break;
                case "finalpayment":
                    targetBudget.setType(2);
                    break;
            }
            targetBudget.setStage(orginBuget.getStage());
            targetBudget.setAmount(orginBuget.getAmount());
            bugetList.add(targetBudget);
        }
        targetEntity.setBudgets(bugetList);
        targetEntity.setRecipient(data.getRecipient());
    }

    private void setWebData(SuggestBean suggestBean) {
        SuggestBean.DataBean data = suggestBean.getData();
        tvSuggestTile.setText(data.getTitle());
        tvNum.setText("#" + data.getId());
        tvTime.setText(DateUtil.timeNYR(data.getCreatedAt(), getContext(), true));
        tvDid.setText(data.getDid());
        tvPeople.setText(data.getDidName());
        setRecycleView();
        tvAbs.setText(data.getAbs());

    }

    private void setRecycleView() {
        List<RecieveProposalJwtEntity.DataBean.BudgetsBean> list = entity.getData().getBudgets();

        if (list == null || list.size() == 0) {
            tvTagMoney.setVisibility(View.GONE);
            rv.setVisibility(View.GONE);
            return;
        }
        BigDecimal sum = new BigDecimal(0);
        for (RecieveProposalJwtEntity.DataBean.BudgetsBean bean : list) {
            sum = sum.add(new BigDecimal(bean.getAmount()));
        }
        tvCount.setText(NumberiUtil.salaToEla(sum) + " ELA");
        SuggetMoneyRecAdapetr adapter = new SuggetMoneyRecAdapetr(getContext(), list);
        rv.setLayoutManager(new LinearLayoutManager(getContext()));
        rv.setAdapter(adapter);
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

        switch (methodName) {
            case "getSuggestion":
                setWebData((SuggestBean) baseEntity);
                break;
            case "newPublishTransaction":

                String hash = "";
                try {
                    JSONObject pulishdata = new JSONObject(((CommmonStringWithiMethNameEntity) baseEntity).getData());
                    hash = pulishdata.getString("TxHash");
                } catch (JSONException e) {
                    e.printStackTrace();
                }
                post(RxEnum.TRANSFERSUCESS.ordinal(), 37 + "", hash);
                presenter.calculateProposalHash(wallet.getWalletId(), new Gson().toJson(targetEntity), this, payPasswd);

                break;
            case "signTransaction":
                new PwdPresenter().newPublishTransaction(wallet.getWalletId(), MyWallet.ELA, ((CommmonStringWithiMethNameEntity) baseEntity).getData(), this);

                break;
            case "createProposalTransaction":
                //这里直接签名发交易   已经输过密码了
                new PwdPresenter().signTransaction(wallet.getWalletId(), MyWallet.ELA, ((CommmonStringEntity) baseEntity).getData(), (String) o, this);

                break;
            case "calculateProposalHash":
                backProposalJwt("signature", ((CommmonStringEntity) baseEntity).getData());
                break;
            case "proposalCRCouncilMemberDigest":
                String signDigest1 = getSignDigist(payPasswd, ((CommmonStringEntity) baseEntity).getData());
                //生成transactionPayload
                targetEntity.setCRCouncilMemberSignature(signDigest1);
                presenter.createProposalTransaction(wallet.getWalletId(), new Gson().toJson(targetEntity), this, payPasswd);

                break;
            case "proposalOwnerDigest":
                //didsdk签名
                String signDigest = getSignDigist(payPasswd, ((CommmonStringEntity) baseEntity).getData());
                backProposalJwt("signature", signDigest);
                break;
            case "postData":
                String des = "";
                if ("createsuggestion".equals(command)) {
                    des = getString(R.string.signsendsuccess);

                }
                new DialogUtil().showTransferSucess(des, getBaseActivity(), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        pop();
                    }
                });

                break;
            case "exportxPrivateKey":
                String privateKey = ((CommmonStringEntity) baseEntity).getData();
                try {
                    store.initPrivateIdentity(privateKey, payPasswd);
                    containsPrivateIdentity();
                } catch (DIDException e) {
                    e.printStackTrace();
                    showToast(getString(R.string.didinitfaile));
                }
                break;
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();

        if (integer == RxEnum.VERTIFYPAYPASS.ordinal()) {
            //验证密码成功
            payPasswd = (String) result.getObj();
            initDid();
        }
        if (integer == RxEnum.JUSTSHOWFEE.ordinal()) {
            //展示手续费后  再去验证密码
            toVertifyPwdActivity();

        }

    }

    private void toVertifyPwdActivity() {
        Intent intent = new Intent(getActivity(), VertifyPwdActivity.class);
        intent.putExtra("walletId", wallet.getWalletId());
        intent.putExtra("type", this.getClass().getSimpleName());
        startActivity(intent);
    }

    private String getSignDigist(String payPasswd, String digist) {

        try {
            String sign = getMyDID().getDIDDocument().signDigest(payPasswd, JwtUtils.hex2byteBe(digist));
            Log.i("signDigest", sign);
            return JwtUtils.bytesToHexString(Base64.decode(sign, Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP));

        } catch (DIDStoreException e) {
            e.printStackTrace();
        }
        return null;

    }


    private void backProposalJwt(String type, String data) {
        ProposalCallBackEntity callBackJwtEntity = new ProposalCallBackEntity();
        callBackJwtEntity.setType(type);
        callBackJwtEntity.setIss(getMyDID().getDidString());
        callBackJwtEntity.setIat(new Date().getTime() / 1000);
        callBackJwtEntity.setExp(new Date().getTime() / 1000 + 10 * 60);
        callBackJwtEntity.setAud(entity.getIss());
        callBackJwtEntity.setReq(scanResult);
        callBackJwtEntity.setCommand(entity.getCommand());
        callBackJwtEntity.setData(data);
        String header = JwtUtils.getJwtHeader(scanResult.replace("elastos://crproposal/", ""));
        // Base64
        String payload = Base64.encodeToString(JSON.toJSONString(callBackJwtEntity).getBytes(), Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
        payload = payload.replaceAll("=", "");
        try {
            String signature = getMyDID().getDIDDocument().sign(payPasswd, (header + "." + payload).getBytes());
            new AuthorizationPresenter().postData(entity.getCallbackurl(), header + "." + payload + "." + signature, this);
        } catch (DIDStoreException e) {
            e.printStackTrace();
        }
    }

    private void containsPrivateIdentity() {
        getMyDID().initDID(payPasswd);
        targetEntity = new ProposalCRCouncialMenberDigestPayLoad();
        ConverProposalOwnerDigestPayLoad(entity);
        if ("createsuggestion".equals(command)) {
            //spv签名
            presenter.proposalOwnerDigest(wallet.getWalletId(), new Gson().toJson(targetEntity), this);
        } else if ("createproposal".equals(command)) {
            targetEntity.setSignature(entity.getData().getSignature());
            targetEntity.setCRCouncilMemberDID(wallet.getDid().replace("did:elastos:", ""));
            presenter.proposalCRCouncilMemberDigest(wallet.getWalletId(), new Gson().toJson(targetEntity), this);

        }
    }

    private void initDid() {
        try {
            store = getMyDID().getDidStore();
            if (store.containsPrivateIdentity()) {
                containsPrivateIdentity();
            } else {
                //获得私钥用于初始化did
                new CreatMulWalletPresenter().exportxPrivateKey(wallet.getWalletId(), payPasswd, this);
            }
        } catch (DIDException e) {
            e.printStackTrace();
        }

    }


}
