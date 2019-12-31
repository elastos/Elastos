package org.elastos.wallet.ela.ui.common.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.common.bean.CommmonObjectWithMethNameEntity;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonObjectWithMethNameViewData;

public class ServerCommonObjectWithMNListener extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        CommmonObjectWithMethNameViewData viewData = (CommmonObjectWithMethNameViewData) getViewData();
        viewData.onGetCommonData((String) getObj(), t);

    }
}
