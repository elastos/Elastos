package org.elastos.wallet.ela.ui.Assets.listener;

import android.view.View;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonCreateSubWalletViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;

public class CreateSubWalletListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {

        (  (CommonCreateSubWalletViewData) getViewData()).onCreateSubWallet(((CommmonStringEntity)t).getData(), (View) getObj());
    }
}
