package org.elastos.wallet.ela;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.listener.CommonObjectWithMethNameListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class FirstPresenter extends PresenterAbstract {


    public void getAllMasterWallets(BaseFragment baseFragment) {
        Observer observer = createObserver(CommonObjectWithMethNameListener.class, baseFragment, false);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllMasterWallets();
            }
        });
        subscriberObservable(observer, observable);
    }

    public void getAllMasterWalletIds(BaseFragment baseFragment) {
        Observer observer = createObserver(CommonObjectWithMethNameListener.class, baseFragment, false);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllMasterWalletIds();
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getMasterWalletBaseEntity(String walletId, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonObjectWithMethNameListener.class, baseFragment, false);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getMasterWalletBaseEntity(walletId);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }


}
