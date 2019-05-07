package org.elastos.wallet.ela.ui.common.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonGetTransactionViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringViewData;

public class CommonStringListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        CommmonStringViewData viewData = (CommmonStringViewData) getViewData();
        viewData.onGetCommonData(((CommmonStringEntity) t).getData());
    }
}
