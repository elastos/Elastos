package org.elastos.wallet.ela;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.listener.CommonStringListListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class FirstPresenter extends PresenterAbstract {



    public void getAllMasterWallets( BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringListListener.class, baseFragment,false);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllMasterWallets();
            }
        });
        subscriberObservable(observer, observable);
    }


}
