package org.elastos.wallet.ela.ui.Assets.viewdata;

import org.elastos.wallet.ela.rxjavahelp.BaseViewData;

public interface WalletManageViewData extends BaseViewData {
    void onOutportMnemonic(String data);
    void onDestoryWallet(String data);

}
