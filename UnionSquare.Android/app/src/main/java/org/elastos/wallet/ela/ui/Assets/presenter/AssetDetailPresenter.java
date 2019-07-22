package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.GetAllTransactionListner;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class AssetDetailPresenter extends PresenterAbstract {


    public void getOwnerAddress(String walletId, String chainID, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getOwnerAddress(walletId, chainID);
            }
        });
        subscriberObservable(observer, observable,baseFragment);
    }


    public void getAllCoinBaseTransaction(String walletId, String chainId, int start, int count, String s, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllCoinBaseTransaction(walletId, chainId, start, count, s);
            }
        });
        subscriberObservable(observer, observable,baseFragment);
    }

    public void createCombineUTXOTransaction(String walletId, String chainId, String memo, boolean useVotedUTXO, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createCombineUTXOTransaction(walletId, chainId, memo, useVotedUTXO);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getAllUTXOs(String walletId, String chainId, int start, int count, String address, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllUTXOs(walletId, chainId, start, count, address);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }
}
