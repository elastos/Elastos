package org.elastos.wallet.ela.ui.did.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class AddDIDPresenter extends NewPresenterAbstract {


    public void getAllPublicKeys(String walletId, String chainID, int start, int count, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getAllPublicKeys", walletId);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllPublicKeys(walletId, chainID, start, count);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }


    public void getDIDByPublicKey(String masterWalletID, String publicKey, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getDIDByPublicKey");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getDIDByPublicKey(masterWalletID, publicKey);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }


    public void getAllSubWallets(String walletId, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getAllSubWallets", walletId);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllSubWallets(walletId);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    /**
     * 为了逻辑清晰使用不同方法名处理不同位置调用后的结果
     *
     * @param walletId
     * @param baseFragment
     */
    public void getAllSubWallets1(String walletId, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getAllSubWallets1", walletId);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllSubWallets(walletId);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }
}
