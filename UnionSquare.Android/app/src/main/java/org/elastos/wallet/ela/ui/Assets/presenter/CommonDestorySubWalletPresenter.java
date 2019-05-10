package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.DestorySubWalletListner;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CommonDestorySubWalletPresenter extends PresenterAbstract {
    public void destroySubWallet(String masterWalletID, String chainID, BaseFragment baseFragment) {
        Observer observer = createObserver(DestorySubWalletListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().destroySubWallet(masterWalletID, chainID);
            }
        });
        subscriberObservable(observer, observable);

    }
}
