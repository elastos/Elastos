package org.elastos.wallet.ela.ui.common.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.common.bean.CommmonObjEntity;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonObjViewData;

public class CommonObjListener extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        CommmonObjViewData viewData = (CommmonObjViewData) getViewData();
        viewData.onGetCommonObjData(((CommmonObjEntity) t).getData());
    }
}