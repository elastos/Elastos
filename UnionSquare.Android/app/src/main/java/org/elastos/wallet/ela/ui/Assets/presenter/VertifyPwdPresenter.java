package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class VertifyPwdPresenter extends NewPresenterAbstract {
    public void verifyPayPassword(String walletId, String payPasswd, BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "verifyPayPassword",payPasswd);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().verifyPayPassword(walletId, payPasswd);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }
}
