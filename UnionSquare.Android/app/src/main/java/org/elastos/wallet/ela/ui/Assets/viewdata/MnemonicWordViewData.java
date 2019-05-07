package org.elastos.wallet.ela.ui.Assets.viewdata;

import org.elastos.wallet.ela.rxjavahelp.BaseViewData;

public interface MnemonicWordViewData extends BaseViewData {
    void onGetMneonic(String data);

    void onCreateMasterWallet(String data);

}
