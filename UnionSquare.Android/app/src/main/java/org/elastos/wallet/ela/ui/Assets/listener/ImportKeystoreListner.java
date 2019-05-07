package org.elastos.wallet.ela.ui.Assets.listener;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.Assets.viewdata.ImportKeystoreViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.ImportMnemonicViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;

public class ImportKeystoreListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        ImportKeystoreViewData viewData = (ImportKeystoreViewData) getViewData();
        viewData.onImportKeystore(((CommmonStringEntity) t).getData());
    }
}
