package org.elastos.wallet.ela.ui.Assets.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.UpdataWalletPwdViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;

public class UpdataWalletPwdListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        UpdataWalletPwdViewData viewData = (UpdataWalletPwdViewData) getViewData();
        viewData.onUpdataPwd(((CommmonStringEntity) t).getData());
    }
}
