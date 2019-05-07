package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.GetSupportedChainsListner;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class AddAssetPresenter extends PresenterAbstract {



    public void getSupportedChains(String walletId, BaseFragment baseFragment) {
        Observer observer = createObserver(GetSupportedChainsListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getSupportedChains(walletId);
            }
        });
        subscriberObservable(observer, observable);
    }


}
