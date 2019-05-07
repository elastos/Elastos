package org.elastos.wallet.ela.ui.Assets.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonCreateSubWalletViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonDestorySubWalletViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;

public class DestorySubWalletListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {

        (  (CommonDestorySubWalletViewData) getViewData()).onDestorySubWallet(((CommmonStringEntity)t).getData());
    }
}
