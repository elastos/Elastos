package org.elastos.wallet.ela.ui.proposal.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.did.exception.DIDStoreException;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.bean.qr.CreateProposalJwtEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalPresenter;
import org.elastos.wallet.ela.ui.proposal.presenter.bean.ProposalOwnerDigestPayLoad;
import org.elastos.wallet.ela.ui.vote.activity.VertifyPwdActivity;
import org.elastos.wallet.ela.utils.JwtUtils;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
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
    private CreateProposalJwtEntity entity;
    private String digist;

    @Override
    protected void setExtraData(Bundle data) {
        wallet = data.getParcelable("wallet");
        entity = data.getParcelable("JwtEntity");
    }

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_proposal_suggest;
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.suggest);
        //todo 获取数据
        registReceiver();
    }

    @OnClick({R.id.tv_sign})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sign:
                //签名
                ProposalOwnerDigestPayLoad payLoad = ConverProposalOwnerDigestPayLoad(entity);
                if (payLoad == null) {
                    showToast(getString(R.string.infoformatwrong));
                } else {

                    new ProposalPresenter().proposalOwnerDigest(wallet.getWalletId(), JSON.toJSONString(payLoad), this);
                }

                break;


        }
    }

    private ProposalOwnerDigestPayLoad ConverProposalOwnerDigestPayLoad(CreateProposalJwtEntity entity) {
        CreateProposalJwtEntity.DataBean data = entity.getData();
        ProposalOwnerDigestPayLoad targetEntity = new ProposalOwnerDigestPayLoad();
        String originType = data.getProposaltype().toLowerCase();
        if ("Normal".toLowerCase().equals(originType)) {
            targetEntity.setType(0x0000);
        } else if ("ELIP".toLowerCase().equals(originType)) {
            targetEntity.setType(0x0100);
        } else {
            return null;
        }

        targetEntity.setCategoryData(data.getCategorydata());
        targetEntity.setOwnerPublicKey(data.getOwnerpublickey());
        targetEntity.setDraftHash(data.getDrafthash());
        List<ProposalOwnerDigestPayLoad.BudgetsBean> bugetList = new ArrayList<>();
        for (CreateProposalJwtEntity.DataBean.BudgetsBean orginBuget : data.getBudgets()) {
            ProposalOwnerDigestPayLoad.BudgetsBean targetBudget = new ProposalOwnerDigestPayLoad.BudgetsBean();
            switch (orginBuget.getType().toLowerCase()) {
                case "imprest":
                    targetBudget.setType(0);
                    break;
                case "normalPayment":
                    targetBudget.setType(1);
                    break;
                case "finalPayment":
                    targetBudget.setType(2);
                    break;
                default:
                    return null;
            }
            targetBudget.setStage(orginBuget.getStage());
            targetBudget.setAmount(orginBuget.getAmount());
            bugetList.add(targetBudget);
        }
        targetEntity.setBudgets(bugetList);
        targetEntity.setRecipient(data.getRecipient());
        return targetEntity;
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
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
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();

        if (integer == RxEnum.VERTIFYPAYPASS.ordinal()) {
            //验证密码成功
            didSign((String) result.getObj());
        }

    }

    private void didSign(String pwd) {
        try {
            String sign = getMyDID().getDIDDocument().signDigest(pwd, JwtUtils.hex2byte(digist));
            Log.i("???", sign);
            showToast(sign);
        } catch (DIDStoreException e) {
            e.printStackTrace();
        }
    }
}
