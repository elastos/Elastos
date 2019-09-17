package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.GetBalanceListner;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CommonGetBalancePresenter extends PresenterAbstract {


    public void getBalance(String walletId, String chainID, int BalanceType, BaseFragment baseFragment) {
        Observer observer = createObserver(GetBalanceListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getBalance(walletId, chainID);
            }
        });
        subscriberObservable(observer, observable);
    }
}
