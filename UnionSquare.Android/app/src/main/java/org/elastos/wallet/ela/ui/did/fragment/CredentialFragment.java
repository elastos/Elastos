package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.view.View;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.did.entity.DIDListEntity;

public  class CredentialFragment extends BaseFragment {
    private DIDListEntity.DIDBean didInfo;
    @Override
    protected int getLayoutId() {
        return R.layout.fragment_did_credential;
    }

    @Override
    protected void setExtraData(Bundle data) {
        didInfo = data.getParcelable("didInfo");
    }

    @Override
    protected void initView(View view) {

    }
}
