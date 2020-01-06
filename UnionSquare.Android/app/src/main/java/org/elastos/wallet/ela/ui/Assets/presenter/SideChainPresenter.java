package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.listener.CommonBooleanListener;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class SideChainPresenter extends PresenterAbstract {



    public void createDepositTransaction(String masterWalletID, String chainID, String fromAddress, String sideChainID, String amount
            , String sideChainAddress, String memo, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createDepositTransaction(masterWalletID, chainID, fromAddress, sideChainID, amount
                        , sideChainAddress, memo);
            }
        });
        subscriberObservable(observer, observable);
    }


    public void createWithdrawTransaction(String walletId, String chainId, String fromAddress, String actualSpend, String address, String memo, BaseFragment baseFragment) {

        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createWithdrawTransaction(walletId, chainId, fromAddress, actualSpend, address, memo);
            }
        });
        subscriberObservable(observer, observable);
    }
}
