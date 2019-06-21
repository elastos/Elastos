package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.listener.CommonBooleanListener;
import org.elastos.wallet.ela.ui.common.listener.CommonLongListener;
import org.elastos.wallet.ela.ui.common.listener.CommonStringListner;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class TransferPresenter extends PresenterAbstract {


    public void isAddressValid(String walletId, String addr, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonBooleanListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().isAddressValid(walletId, addr);
            }
        });
        subscriberObservable(observer, observable);
    }


    public void createTransaction(String walletId, String chainId, String s, String address, long amount, String memo, boolean useVotedUTXO,BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createTransaction(walletId, chainId, s, address, amount, memo, useVotedUTXO);
            }
        });
        subscriberObservable(observer, observable);
    }

}