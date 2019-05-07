package org.elastos.wallet.ela.ui.Assets.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.viewdata.AssetsViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonObjEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;

public class GetBalanceListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        CommonBalanceViewData viewData = (CommonBalanceViewData) getViewData();
        viewData.onBalance((BalanceEntity) ((CommmonObjEntity) t).getData());
    }
}
