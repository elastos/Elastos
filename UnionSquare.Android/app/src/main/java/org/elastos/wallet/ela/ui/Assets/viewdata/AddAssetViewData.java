package org.elastos.wallet.ela.ui.Assets.viewdata;

import org.elastos.wallet.ela.rxjavahelp.BaseViewData;

public interface AddAssetViewData extends BaseViewData {
    void onGetSupportedChains(String[] data);
}
