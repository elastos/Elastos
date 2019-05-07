package org.elastos.wallet.ela.ui.common.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonBooleanViewData;

public class CommonBooleanListener extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        CommmonBooleanViewData viewData = (CommmonBooleanViewData) getViewData();
        viewData.onGetCommonData(((CommmonBooleanEntity) t).getData());
    }
}
