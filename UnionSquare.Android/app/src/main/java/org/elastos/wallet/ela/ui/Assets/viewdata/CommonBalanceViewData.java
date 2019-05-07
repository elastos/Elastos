package org.elastos.wallet.ela.ui.Assets.viewdata;

import org.elastos.wallet.ela.rxjavahelp.BaseViewData;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;

public interface CommonBalanceViewData extends BaseViewData {


    void onBalance(BalanceEntity data);

}
