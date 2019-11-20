package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.ui.Assets.listener.GetAllSubWalletsListner;
import org.elastos.wallet.ela.ui.Assets.listener.ISubWalletListener;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class AssetsPresenter extends NewPresenterAbstract {

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
        subscriberObservable(observer, observable, baseFragment);
    }

    public void syncStart(String walletId, String chainId, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().syncStart(walletId, chainId);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void syncStop(String walletId, String chainId, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().syncStop(walletId, chainId);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void setFixedPeer(String walletId, String chainId, String address, int port, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "setFixedPeer");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().setFixedPeer(walletId, chainId, address, port);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }


}
