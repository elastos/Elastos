package org.elastos.wallet.ela.ui.Assets.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.AddAssetViewData;
import org.elastos.wallet.ela.ui.common.bean.CommonStringArrayEntity;

public class GetSupportedChainsListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        AddAssetViewData viewData = (AddAssetViewData) getViewData();
        viewData.onGetSupportedChains(((CommonStringArrayEntity) t).getData());
    }
}
