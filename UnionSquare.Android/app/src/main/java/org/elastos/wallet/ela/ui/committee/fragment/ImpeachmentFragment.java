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

package org.elastos.wallet.ela.ui.committee.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.activity.TransferActivity;
import org.elastos.wallet.ela.ui.committee.bean.CtListBean;
import org.elastos.wallet.ela.ui.committee.presenter.CtListPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonLongEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.NumberiUtil;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * impeachment fragment(input votes, verify password, prompt success)
 */
public class ImpeachmentFragment extends BaseFragment implements NewBaseViewData {

    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    private CtListPresenter presenter;
    @BindView(R.id.next_step_layout)
    View nextLayout;
    @BindView(R.id.confirm_layout)
    View confirmLayout;
    String cid;

    @BindView(R.id.fee)
    TextView feeTv;
    @BindView(R.id.impeachvotes)
    TextView amountTv;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_impeachment;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        cid = data.getString("cid");
    }

    @Override
    protected void initView(View view) {
        new CRSignUpPresenter().getFee(wallet.getWalletId(), MyWallet.ELA, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", this);
        presenter = new CtListPresenter();
//        presenter.getVoteInfo(wallet.getWalletId(), MyWallet.ELA, "CRC", this);
    }

    @OnClick({R.id.close, R.id.next_step_btn, R.id.confirm_btn})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.close:
                popBackFragment();
                break;
            case R.id.next_step_btn:
                nextLayout.setVisibility(View.GONE);
                confirmLayout.setVisibility(View.VISIBLE);
                break;
            case R.id.confirm_btn:
                impeachCt();
                break;
        }
    }

    private void impeachCt() {
        presenter.getCouncilList(this, String.valueOf(1));
    }

    JSONArray otherUnActiveVote = new JSONArray();
    private void createJsonObject(CtListBean ctListBean) {
        List<CtListBean.Council> councils = ctListBean.getData().getCouncil();
        JSONObject unActiveVotes = new JSONObject();
        List<String> candidates = new ArrayList<>();
        if(null==councils || councils.size()<=0) return;
        for(CtListBean.Council council : councils) {
            String status = council.getStatus();
            String did = council.getDid();
            if(AppUtlis.isNullOrEmpty(status) || !status.equals("Elected")) {
                candidates.add(did);
            }
        }

        unActiveVotes.put("Type", "CRCImpeachment");
        unActiveVotes.put("Candidates", JSON.toJSON(candidates));
        otherUnActiveVote.add(unActiveVotes);
    }


    JSONObject otherVotes = new JSONObject();
    private void getVotes(CommmonStringEntity commmonStringEntity) {
        try {
            if(null == commmonStringEntity) return;
            String json = commmonStringEntity.getData();

            JSONArray jsonArray = (JSONArray) JSON.parse(json);
            JSONObject jsonObject = (JSONObject) jsonArray.get(0);
            JSONObject votesObj = (JSONObject) jsonObject.get("Votes");
            String amount = (String) votesObj.get(cid);

            otherVotes.put(cid, amount);
            Log.d("test", "amount:"+amount);
        } catch (Exception e) {
            e.printStackTrace();
        }
//        otherVotes.put(did, amount);
    }

    private void doVote() {
        presenter.createImpeachmentCRCTransaction(wallet.getWalletId(), MyWallet.ELA, "", otherVotes.toJSONString(), "", otherUnActiveVote.toJSONString(), this);
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        if(methodName.equals("getFee")) {
            long fee = ((CommmonLongEntity) baseEntity).getData();
            feeTv.setText(NumberiUtil.maxNumberFormat(Arith.div(fee + "", MyWallet.RATE_S).toPlainString(), 12) + " " + MyWallet.ELA);
        } else if(methodName.equals("getCouncilList")) {
            presenter.getVoteInfo(wallet.getWalletId(), MyWallet.ELA, "CRCImpeachment", this);
            createJsonObject((CtListBean) baseEntity);
        } else if(methodName.equals("getVoteInfo")) {
            getVotes((CommmonStringEntity) baseEntity);
            doVote();
        } else if(methodName.equalsIgnoreCase("createImpeachmentCRCTransaction")) {
            goTransferActivity(((CommmonStringEntity) baseEntity).getData());
        }
    }

    private void goTransferActivity(String attributesJson) {
        Intent intent = new Intent(getActivity(), TransferActivity.class);
        intent.putExtra("amount", amountTv.getText().toString());
        intent.putExtra("wallet", wallet);
        intent.putExtra("chainId", MyWallet.ELA);
        intent.putExtra("attributes", attributesJson);
        intent.putExtra("type", Constant.PROPOSALPUBLISHED);
        intent.putExtra("transType", 1004);
        startActivity(intent);
    }
}
