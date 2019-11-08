package org.elastos.wallet.ela.ui.did.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class DIDListPresenter extends NewPresenterAbstract {


    public void getResolveDIDInfo(String walletId, int start, int count, String did, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getResolveDIDInfo", walletId);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getResolveDIDInfo(walletId, start, count, did);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }
    public void getResolveDIDInfo1(String walletId, int start, int count, String did, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getResolveDIDInfo1", walletId);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getResolveDIDInfo(walletId, start, count, did);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }


}
