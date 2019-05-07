package org.elastos.wallet.ela.ui.Assets.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonGetTransactionViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;

public class GetAllTransactionListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        CommonGetTransactionViewData viewData = (CommonGetTransactionViewData) getViewData();
        viewData.onGetAllTransaction(((CommmonStringEntity) t).getData());
    }
}
