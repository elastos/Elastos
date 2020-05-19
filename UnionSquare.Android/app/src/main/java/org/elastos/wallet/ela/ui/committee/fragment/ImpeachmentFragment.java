package org.elastos.wallet.ela.ui.committee.fragment;

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
import org.elastos.wallet.ela.ui.committee.bean.CtListBean;
import org.elastos.wallet.ela.ui.committee.presenter.CtListPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonLongEntity;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Arith;
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

    @BindView(R.id.fee)
    TextView feeTv;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_impeachment;
    }

    @Override
    protected void initView(View view) {
        new CRSignUpPresenter().getFee(wallet.getWalletId(), MyWallet.ELA, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", this);
    }

    @OnClick({R.id.close, R.id.next_step_btn, R.id.confirm_btn})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.close:
                popBackFragment();
                break;
            case R.id.confirm_btn:
                impeachCt();
                break;
        }
    }

    private void impeachCt() {
        presenter = new CtListPresenter();
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

        unActiveVotes.put("Type", "Delegate");
        unActiveVotes.put("Candidates", JSON.toJSON(candidates));
        otherUnActiveVote.add(unActiveVotes);
    }


    public static  class InfoStruct {
        public String Type;
        public String Amount;
        public String Votes;
    }



    JSONObject otherVotes = new JSONObject();
    private void getVotes(String json) {
        if(AppUtlis.isNullOrEmpty(json)) return;

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
            presenter.getVoteInfo(wallet.getWalletId(), MyWallet.ELA, "CRC", this);
            createJsonObject((CtListBean) baseEntity);
        } else if(methodName.equals("getVoteInfo")) {
            getVotes(baseEntity.getMessage());
            doVote();
        }
    }
}
