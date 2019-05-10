package org.elastos.wallet.ela.ui.Assets.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.OutportKeystoreViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;

public class OutportKeystoreListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        OutportKeystoreViewData viewData = (OutportKeystoreViewData) getViewData();
        viewData.onOutportKeystore(((CommmonStringEntity) t).getData());
    }
}
