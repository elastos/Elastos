package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.UpdataWalletPwdListner;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class WalletUpdatePwdPresenter extends PresenterAbstract {


    public void changePassword(String walletId, String originPwd, String newPwd, BaseFragment baseFragment) {
        Observer observer = createObserver(UpdataWalletPwdListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().changePassword(walletId, originPwd, newPwd);
            }
        });
        subscriberObservable(observer, observable);
    }
}
