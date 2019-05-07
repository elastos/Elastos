package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.GetAllTransactionListner;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CommonGetTransactionPresenter extends PresenterAbstract {

    public void getAllTransaction(String walletId, String chainId, int start, int count, String s, BaseFragment baseFragment) {
        Observer observer = createObserver(GetAllTransactionListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllTransaction(walletId, chainId, start, count, s);
            }
        });
        subscriberObservable(observer, observable);
    }
}
