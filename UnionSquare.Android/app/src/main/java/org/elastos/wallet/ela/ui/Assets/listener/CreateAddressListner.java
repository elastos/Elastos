package org.elastos.wallet.ela.ui.Assets.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.ReceiptViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;

public class CreateAddressListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        ReceiptViewData viewData = (ReceiptViewData) getViewData();
        viewData.onCreateAddress(((CommmonStringEntity) t).getData());
    }
}
