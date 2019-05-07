package org.elastos.wallet.ela.ui.Assets.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.UpdataWalletPwdViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.WalletManageViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;

public class OutportWalletMnemonicListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        WalletManageViewData viewData = (WalletManageViewData) getViewData();
        viewData.onOutportMnemonic(((CommmonStringEntity) t).getData());
    }
}
