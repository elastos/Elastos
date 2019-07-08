package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.GetAllSubWalletsListner;
import org.elastos.wallet.ela.ui.Assets.listener.ISubWalletListener;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class AssetsPresenter extends PresenterAbstract {

    public void registerWalletListener(String masterWalletID, String chainID, BaseFragment baseFragment) {
        baseFragment.getMyWallet().registerWalletListener(masterWalletID, chainID, (ISubWalletListener) baseFragment);

    }

    public void getAllSubWallets(String walletId, BaseFragment baseFragment) {
        Observer observer = createObserver(GetAllSubWalletsListner.class, baseFragment, 0);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllSubWallets(walletId);
            }
        });
        subscriberObservable(observer, observable);
    }

    public void getAllSubWallets(String walletId, int type, BaseFragment baseFragment) {
        Observer observer = createObserver(GetAllSubWalletsListner.class, baseFragment, type);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllSubWallets(walletId);
            }
        });
        subscriberObservable(observer, observable);
    }

    public void syncStart(String walletId, String chainId, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().syncStart(walletId, chainId);
            }
        });
        subscriberObservable(observer, observable);
    }


}
