package org.elastos.wallet.ela.ui.find.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.find.listener.RegisteredProducerInfoListner;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class VoteFirstPresenter extends PresenterAbstract {


    public void getRegisteredProducerInfo(String walletId, String chainID, BaseFragment baseFragment) {
        Observer observer = createObserver(RegisteredProducerInfoListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getRegisteredProducerInfo(walletId, chainID);
            }
        });
        subscriberObservable(observer, observable);
    }


}
