package org.elastos.wallet.ela.ui.main.viewdata;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.rxjavahelp.BaseViewData;

public interface MainViewData extends BaseViewData {
    void onGetMyWallet(MyWallet myWallet);

}

