package org.elastos.wallet.ela.ui.common.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringListEntity;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonBooleanViewData;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringListViewData;

public class CommonStringListListener extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        CommmonStringListViewData viewData = (CommmonStringListViewData) getViewData();
        viewData.onGetStringListCommonData(((CommmonStringListEntity) t).getData());
    }
}
