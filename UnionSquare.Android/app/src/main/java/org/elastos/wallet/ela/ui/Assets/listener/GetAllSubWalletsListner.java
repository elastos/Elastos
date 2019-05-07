package org.elastos.wallet.ela.ui.Assets.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.AssetsViewData;
import org.elastos.wallet.ela.ui.common.bean.ISubWalletListEntity;

public class GetAllSubWalletsListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        AssetsViewData viewData = (AssetsViewData) getViewData();
        viewData.onGetAllSubWallets(((ISubWalletListEntity) t).getData());
    }
}
