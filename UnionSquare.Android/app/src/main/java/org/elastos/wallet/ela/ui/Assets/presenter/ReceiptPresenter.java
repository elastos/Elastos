package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.CreateAddressListner;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class ReceiptPresenter extends PresenterAbstract {



    public void createAddress(String walletId, String chainId,BaseFragment baseFragment) {
        Observer observer = createObserver(CreateAddressListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createAddress(walletId,chainId);
            }
        });
        subscriberObservable(observer, observable);
    }


}
