package org.elastos.wallet.ela.ui.vote.voteFirst;


import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetTransactionPresenter;
import org.elastos.wallet.ela.ui.find.presenter.VoteFirstPresenter;
import org.elastos.wallet.ela.ui.find.viewdata.RegisteredProducerInfoViewData;
import org.elastos.wallet.ela.ui.vote.ElectoralAffairs.ElectoralAffairsFragment;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.SuperNodeListFragment;
import org.elastos.wallet.ela.ui.vote.signupfor.SignUpForFragment;

import butterknife.BindView;
import butterknife.OnClick;


public class VoteFirstFragment extends BaseFragment implements RegisteredProducerInfoViewData {


    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_signupfor)
    TextView tv_signupfor;


    private CommonGetTransactionPresenter presenter = new CommonGetTransactionPresenter();
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    VoteFirstPresenter findPresenter = new VoteFirstPresenter();

    private String status;
    private String info;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_vote_first;
    }


    @Override
    protected void initInjector() {

    }

    @Override
    protected void setExtraData(Bundle data) {

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.supernode_election));
        findPresenter.getRegisteredProducerInfo(wallet.getWalletId(), MyWallet.ELA, this);
    }


    @OnClick({R.id.tv_signupfor, R.id.tv_goingtovote})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_signupfor:
                if (tv_signupfor.getText().equals(getString(R.string.sign_up_for))) {
                    start(SignUpForFragment.newInstance());
                } else {
                    Bundle bundle = new Bundle();
                    bundle.putString("status", status);
                    bundle.putString("info", info);
                    start(ElectoralAffairsFragment.class, bundle);
                }

                break;
            case R.id.tv_goingtovote:
                start(SuperNodeListFragment.newInstance());
                break;

        }
    }




    //可能的值为: "Unregistered", "Registered", "Canceled", "ReturnDeposit"分别代表, 从未注册过, 已注册过, 已注销,已注销且已提取
    @Override
    public void onGetRegisteredProducerInfo(String data) {
        JSONObject jsonObject = JSON.parseObject(data);
        status = jsonObject.getString("Status");
        info = jsonObject.getString("Info");
        if (!TextUtils.isEmpty(status)) {
            switch (status) {
                case "Unregistered":
                    tv_signupfor.setText(getString(R.string.sign_up_for));
                    tv_signupfor.setVisibility(View.VISIBLE);
                    break;
                case "ReturnDeposit":
                    tv_signupfor.setVisibility(View.GONE);
                    break;
                case "Canceled":
                    tv_signupfor.setText(getString(R.string.electoral_affairs));
                    tv_signupfor.setVisibility(View.VISIBLE);
                    break;
                case "Registered":
                    tv_signupfor.setText(getString(R.string.electoral_affairs));
                    tv_signupfor.setVisibility(View.VISIBLE);
                    break;

            }

        }
    }
}
