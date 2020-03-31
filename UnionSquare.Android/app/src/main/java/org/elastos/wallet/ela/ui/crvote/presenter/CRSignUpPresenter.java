package org.elastos.wallet.ela.ui.crvote.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CRSignUpPresenter extends NewPresenterAbstract {

    public void getFee(String walletId, String chainId, String s, String address, String amount, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getFee");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getFee(walletId, chainId, s, address, amount);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }public void getFee(String walletId, String chainId, String s, String address, String amount,String type, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getFee",type);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getFee(walletId, chainId, s, address, amount);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }
}
