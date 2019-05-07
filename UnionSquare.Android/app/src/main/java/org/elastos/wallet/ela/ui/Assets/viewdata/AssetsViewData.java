package org.elastos.wallet.ela.ui.Assets.viewdata;

import org.elastos.wallet.ela.rxjavahelp.BaseViewData;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.core.SubWallet;

import java.util.List;

public interface AssetsViewData extends BaseViewData {
    void onGetAllSubWallets(List<SubWallet> data);



}
