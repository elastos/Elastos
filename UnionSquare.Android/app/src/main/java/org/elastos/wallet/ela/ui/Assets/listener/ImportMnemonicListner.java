package org.elastos.wallet.ela.ui.Assets.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.ImportMnemonicViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;

public class ImportMnemonicListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        ImportMnemonicViewData viewData = (ImportMnemonicViewData) getViewData();
        viewData.onImportMnemonic(((CommmonStringEntity) t).getData());
    }
}
