package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.OutportKeystoreListner;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class OutportKeystorePresenter extends PresenterAbstract {
    public void exportWalletWithKeystore(String walletId, String pwd, String payPwd, BaseFragment baseFragment) {
        Observer observer = createObserver(OutportKeystoreListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().exportWalletWithKeystore(walletId, pwd, payPwd);
            }
        });
        subscriberObservable(observer, observable);
    }
}
