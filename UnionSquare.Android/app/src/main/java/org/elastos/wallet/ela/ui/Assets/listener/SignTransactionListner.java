package org.elastos.wallet.ela.ui.Assets.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.PwdViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;

public class SignTransactionListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        PwdViewData viewData = (PwdViewData) getViewData();
        viewData.onSignTransaction(((CommmonStringEntity) t).getData());
    }
}
