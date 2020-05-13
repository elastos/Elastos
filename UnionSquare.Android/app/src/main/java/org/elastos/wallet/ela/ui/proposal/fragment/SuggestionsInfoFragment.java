package org.elastos.wallet.ela.ui.proposal.fragment;

import android.content.Intent;
import android.os.Bundle;
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
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecieveProposalJwtEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.mulwallet.CreatMulWalletPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.did.presenter.AuthorizationPresenter;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalCallBackEntity;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalPresenter;
import org.elastos.wallet.ela.ui.proposal.presenter.bean.ProposalCRCouncialMenberDigestPayLoad;
import org.elastos.wallet.ela.ui.proposal.presenter.bean.ProposalOwnerDigestPayLoad;
import org.elastos.wallet.ela.ui.vote.activity.VertifyPwdActivity;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.JwtUtils;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

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
    @BindView(R.id.type)
    TextView type;
    @BindView(R.id.hash)
    TextView hash;
    @BindView(R.id.did)
    TextView did;
    @BindView(R.id.account)
    TextView account;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.abstract1)
    TextView abstract1;
    @BindView(R.id.tv_sign)
    TextView tvSign;
    Unbinder unbinder;
    private Wallet wallet;
    private String scanResult;
    private RecieveProposalJwtEntity entity;
    private String digist;
    private DIDStore store;
    String command;
    ProposalCRCouncialMenberDigestPayLoad targetEntity;

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
        if (command.equals("createsuggestion"))
            tvTitle.setText(R.string.suggest);
        else
            tvTitle.setText(R.string.putproposal);
        //todo 获取数据
        registReceiver();

    }

    @OnClick({R.id.tv_sign})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sign:
                //签名
                if ("createsuggestion".equals(command)) {
                    //给建议签名
                    targetEntity = new ProposalCRCouncialMenberDigestPayLoad();
                    ConverProposalOwnerDigestPayLoad(entity);
                    if (targetEntity == null) {
                        showToast(getString(R.string.infoformatwrong));
                    } else {
                        new ProposalPresenter().proposalOwnerDigest(wallet.getWalletId(), new Gson().toJson(targetEntity), this);
                    }

                } else if ("createproposal".equals(command)) {
                    //发送提案交易
                    targetEntity = new ProposalCRCouncialMenberDigestPayLoad();
                    ConverProposalOwnerDigestPayLoad(entity);
                    targetEntity.setSignature(entity.getData().getSignature());
                    targetEntity.setCRCouncilMemberDID(wallet.getDid().replace("did:elastos:", ""));
                    new ProposalPresenter().proposalCRCouncilMemberDigest(wallet.getWalletId(), new Gson().toJson(targetEntity), this);


                }

                break;


        }
    }


    private void ConverProposalOwnerDigestPayLoad(RecieveProposalJwtEntity entity) {
        RecieveProposalJwtEntity.DataBean data = entity.getData();
        //ProposalOwnerDigestPayLoad targetEntity = new ProposalOwnerDigestPayLoad();
        String originType = data.getProposaltype().toLowerCase();
        if ("Normal".toLowerCase().equals(originType)) {
            targetEntity.setType(0x0000);
        } else if ("ELIP".toLowerCase().equals(originType)) {
            targetEntity.setType(0x0100);
        } else {
            //return null;
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
                /*default:
                    return null;*/
            }
            targetBudget.setStage(orginBuget.getStage());
            targetBudget.setAmount(orginBuget.getAmount());
            bugetList.add(targetBudget);
        }
        targetEntity.setBudgets(bugetList);
        targetEntity.setRecipient(data.getRecipient());
        //return targetEntity;
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "createProposalTransaction":
       /*         Intent intent = new Intent(getActivity(), TransferActivity.class);
                intent.putExtra("amount", amount);
                intent.putExtra("toAddress", address);
                intent.putExtra("wallet", wallet);
                intent.putExtra("chainId", chainId);
                intent.putExtra("attributes", data);
                intent.putExtra("type", Constant.TRANFER);
                intent.putExtra("transType", 2);
                startActivity(intent);*/
                CommmonStringEntity commonEntity1 = (CommmonStringEntity) baseEntity;
                Log.i("???", commonEntity1.getData());
                break;
            case "proposalCRCouncilMemberDigest":
            case "proposalOwnerDigest":
                CommmonStringEntity commonEntity = (CommmonStringEntity) baseEntity;
                digist = commonEntity.getData();
                //didsdk签名
                //getMyDID().getDIDDocument()
                Intent intent = new Intent(getActivity(), VertifyPwdActivity.class);
                intent.putExtra("walletId", wallet.getWalletId());
                intent.putExtra("type", this.getClass().getSimpleName());
                startActivity(intent);
                break;
            case "postData":
                showToast(getString(R.string.authoriizesuccess));

                new DialogUtil().showTransferSucess(getString(R.string.signsendsuccess), getBaseActivity(), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        pop();
                    }
                });

                break;
            case "exportxPrivateKey":
                String privateKey = ((CommmonStringEntity) baseEntity).getData();
                String payPasswd = (String) o;
                try {
                    store.initPrivateIdentity(privateKey, payPasswd);
                    generBackJwt(payPasswd);
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
            initDid((String) result.getObj());
        }

    }

    private String getSignDigist(String payPasswd, String digist) {

        try {
            String sign = getMyDID().getDIDDocument().signDigest(payPasswd, JwtUtils.hex2byte(digist));
            Log.i("signDigest", sign);
            return JwtUtils.bytesToHexString(Base64.decode(sign, Base64.URL_SAFE));

        } catch (DIDStoreException e) {
            e.printStackTrace();
        }
        return null;

    }

    private void generBackJwt(String payPasswd) {
        getMyDID().initDID(payPasswd);
        //把digist diaoyongsdk签名

        String signDigest = getSignDigist(payPasswd, digist);
        if ("createsuggestion".equals(command)) {
            backSuggestJwt(payPasswd, signDigest);
        } else if ("createproposal".equals(command)) {
            //生成transactionPayload
            targetEntity.setCRCouncilMemberSignature(signDigest);
            new ProposalPresenter().createProposalTransaction(wallet.getWalletId(), new Gson().toJson(targetEntity), this);

        }


    }

    private void backSuggestJwt(String payPasswd, String signDigest) {
        ProposalCallBackEntity callBackJwtEntity = new ProposalCallBackEntity();
        callBackJwtEntity.setType("signature");
        callBackJwtEntity.setIss(getMyDID().getDidString());
        callBackJwtEntity.setIat(new Date().getTime() / 1000);
        callBackJwtEntity.setExp(new Date().getTime() / 1000 + 10 * 60);
        callBackJwtEntity.setAud(entity.getIss());
        callBackJwtEntity.setReq(scanResult);
        callBackJwtEntity.setCommand(entity.getCommand());
        callBackJwtEntity.setData(signDigest);
        String header = JwtUtils.getJwtHeader(scanResult.replace("elastos://crproposal/", ""));
        // Base64
        String payload = Base64.encodeToString(JSON.toJSONString(callBackJwtEntity).getBytes(), Base64.URL_SAFE | Base64.NO_WRAP);
        payload = payload.replaceAll("=", "");
        try {
            String signature = getMyDID().getDIDDocument().sign(payPasswd, (header + "." + payload).getBytes());
            new AuthorizationPresenter().postData(entity.getCallbackurl(), header + "." + payload + "." + signature, this);
        } catch (DIDStoreException e) {
            e.printStackTrace();
        }
    }

    private void initDid(String payPasswd) {
        try {
            store = getMyDID().getDidStore();
            if (store.containsPrivateIdentity()) {
                generBackJwt(payPasswd);
            } else {
                //获得私钥用于初始化did
                new CreatMulWalletPresenter().exportxPrivateKey(wallet.getWalletId(), payPasswd, this);
            }
        } catch (DIDException e) {
            e.printStackTrace();
        }

    }

}
