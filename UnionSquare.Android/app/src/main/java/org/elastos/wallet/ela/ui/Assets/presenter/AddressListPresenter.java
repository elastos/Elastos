package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.listener.CommonStringListner;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class AddressListPresenter extends PresenterAbstract {


    public void getAllAddress(String walletId, String chainId, int start, int count, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllAddress(walletId, chainId, start, count);
            }
        });
        subscriberObservable(observer, observable);
    }


}
