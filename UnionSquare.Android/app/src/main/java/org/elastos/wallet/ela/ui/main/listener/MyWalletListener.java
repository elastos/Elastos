package org.elastos.wallet.ela.ui.main.listener;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.common.bean.CommmonObjEntity;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonObjViewData;
import org.elastos.wallet.ela.ui.main.viewdata.MainViewData;

public class MyWalletListener extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        MainViewData viewData = (MainViewData) getViewData();
        viewData.onGetMyWallet((MyWallet) ((CommmonObjEntity) t).getData());
    }
}