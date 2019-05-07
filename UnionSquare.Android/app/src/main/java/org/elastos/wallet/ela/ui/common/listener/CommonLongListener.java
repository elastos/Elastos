package org.elastos.wallet.ela.ui.common.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.common.bean.CommmonLongEntity;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonLongViewData;

public class CommonLongListener extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        CommmonLongViewData viewData = (CommmonLongViewData) getViewData();
        viewData.onGetCommonData(((CommmonLongEntity) t).getData());
    }
}
