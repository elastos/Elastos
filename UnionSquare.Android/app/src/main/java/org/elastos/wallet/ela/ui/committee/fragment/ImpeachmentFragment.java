package org.elastos.wallet.ela.ui.committee.fragment;

import android.view.View;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;

import butterknife.OnClick;

/**
 * impeachment fragment(input votes, verify password, prompt success)
 */
public class ImpeachmentFragment extends BaseFragment implements NewBaseViewData {

    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();

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

                break;
            case R.id.confirm_btn:

                break;
        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

    }
}
