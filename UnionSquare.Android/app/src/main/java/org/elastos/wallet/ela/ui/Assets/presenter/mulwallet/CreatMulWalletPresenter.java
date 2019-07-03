package org.elastos.wallet.ela.ui.Assets.presenter.mulwallet;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.CreateMasterWalletListner;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CreatMulWalletPresenter extends PresenterAbstract {
    public void exportxPrivateKey(String masterWalletID, String payPassword, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().exportxPrivateKey(masterWalletID,
                        payPassword);
            }
        });
        subscriberObservable(observer, observable, baseFragment);

    }


}
