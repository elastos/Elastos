package org.elastos.wallet.ela.ui.common.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringWithiMethNameEntity;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;

public class CommonStringWithiMethNameListener extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        CommmonStringWithMethNameViewData viewData = (CommmonStringWithMethNameViewData) getViewData();
        viewData.onGetCommonData(((CommmonStringWithiMethNameEntity) t).getMethodName(), ((CommmonStringWithiMethNameEntity) t).getData());
    }
}
