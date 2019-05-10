package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.ImportKeystoreListner;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class ImportKeystorePresenter extends PresenterAbstract {


    public void importWalletWithKeystore(String masterWalletID, String keystore, String keystorePwd, String payPassword, BaseFragment baseFragment) {
        Observer observer = createObserver(ImportKeystoreListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().importWalletWithKeystore(masterWalletID, keystore, keystorePwd,
                        payPassword);
            }
        });
        subscriberObservable(observer, observable);
    }


}
