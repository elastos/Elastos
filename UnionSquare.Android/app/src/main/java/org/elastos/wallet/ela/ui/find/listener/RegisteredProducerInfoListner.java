package org.elastos.wallet.ela.ui.find.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.find.viewdata.RegisteredProducerInfoViewData;

public class RegisteredProducerInfoListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        RegisteredProducerInfoViewData viewData = (RegisteredProducerInfoViewData) getViewData();
        viewData.onGetRegisteredProducerInfo(((CommmonStringEntity) t).getData());
    }
}
