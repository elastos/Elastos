package org.elastos.wallet.ela.ui.committee.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.activity.TransferActivity;
import org.elastos.wallet.ela.ui.Assets.presenter.WalletManagePresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.did.fragment.AuthorizationFragment;
import org.elastos.wallet.ela.ui.vote.ElectoralAffairs.ElectoralAffairsPresenter;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.Constant;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * committee manage fragment(in office, expiration office, impeachment)
 */
public class CtManagerFragment extends BaseFragment implements NewBaseViewData {
    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.name)
    TextView name;
    @BindView(R.id.description)
    TextView description;
    @BindView(R.id.refresh_ct_info)
    TextView refreshInfo;
    @BindView(R.id.refresh_ct_did)
    TextView refreshDid;
    @BindView(R.id.deposit)
    TextView deposit;

    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    ElectoralAffairsPresenter presenter = new ElectoralAffairsPresenter();
    String depositAmount;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_manager;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getContext().getString(R.string.ctmanager));
        setView();
    }

    String status;
    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        status = data.getString("status");
    }

    private void setView() {
        switch (status) {
            case "Terminated":
                description.setText(getString(R.string.completeofficehint));
                showDepositView();
                break;
            case "Impeached":
                description.setText(getString(R.string.cancelofficehint));
                showDepositView();
                break;
            case "Returned":
                description.setText(getString(R.string.dimissofficehint));
                showDepositView();
                break;
            case "Elected":
                description.setText(getString(R.string.inofficehint));
                showRefreshView();
                break;
            default:
                throw new IllegalStateException("Unexpected value: " + status);
        }
    }

    private void showRefreshView() {
        refreshInfo.setVisibility(View.GONE);
        refreshDid.setVisibility(View.GONE);
        deposit.setVisibility(View.VISIBLE);
    }

    private void showDepositView() {
        refreshInfo.setVisibility(View.VISIBLE);
        refreshDid.setVisibility(View.VISIBLE);
        deposit.setVisibility(View.GONE);
    }


    @OnClick({R.id.refresh_ct_info, R.id.refresh_ct_did, R.id.deposit})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.refresh_ct_info:
                //TODO 待确认
                start(UpdateCtInformationFragment.class, getArguments());
                new WalletManagePresenter().DIDResolveWithTip(wallet.getDid(), this, "2");
                break;
            case R.id.refresh_ct_did:
                Bundle bundle = new Bundle();
                bundle.putString("type", "authorization");
                bundle.putParcelable("wallet", wallet);
                start(AuthorizationFragment.class, bundle);
                break;
            case R.id.deposit:
                presenter.createRetrieveDepositTransaction(wallet.getWalletId(), MyWallet.ELA,
                        Arith.mulRemoveZero(depositAmount, MyWallet.RATE_S).toPlainString(), this);
                break;
        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "createRetrieveDepositTransaction":
                Intent intent = new Intent(getActivity(), TransferActivity.class);
                intent.putExtra("wallet", wallet);
                intent.putExtra("type", Constant.WITHDRAWSUPERNODE);
                intent.putExtra("amount", depositAmount);
                intent.putExtra("chainId", MyWallet.ELA);
                intent.putExtra("attributes", ((CommmonStringEntity) baseEntity).getData());
                startActivity(intent);
                break;
            case "getDepositcoin":

                break;
        }
    }
}
