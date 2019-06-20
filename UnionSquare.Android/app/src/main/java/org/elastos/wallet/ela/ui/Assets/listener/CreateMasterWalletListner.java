package org.elastos.wallet.ela.ui.Assets.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.CreaterWalletViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;

public class CreateMasterWalletListner extends SubscriberOnNextLisenner {

    @Override
    protected void onNextLisenner(BaseEntity t) {
        CreaterWalletViewData viewData = (CreaterWalletViewData) getViewData();
        viewData.onCreateMasterWallet(((CommmonStringEntity) t).getData());

    }
}
