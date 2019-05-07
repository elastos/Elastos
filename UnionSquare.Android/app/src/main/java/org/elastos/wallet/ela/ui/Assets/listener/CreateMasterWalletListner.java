package org.elastos.wallet.ela.ui.Assets.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.MnemonicWordViewData;

public class CreateMasterWalletListner extends SubscriberOnNextLisenner {

    @Override
    protected void onNextLisenner(BaseEntity t) {
        MnemonicWordViewData viewData = (MnemonicWordViewData) getViewData();
        viewData.onCreateMasterWallet(((CommmonStringEntity)t).getData());

    }
}
