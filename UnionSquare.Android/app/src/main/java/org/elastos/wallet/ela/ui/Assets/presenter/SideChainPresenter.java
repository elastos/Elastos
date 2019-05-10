package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.listener.CommonBooleanListener;
import org.elastos.wallet.ela.ui.common.listener.CommonLongListener;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class SideChainPresenter extends PresenterAbstract {

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

    public void getGenesisAddress(String walletId, String chain, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getGenesisAddress(walletId, chain);
            }
        });
        subscriberObservable(observer, observable);
    }

    public void createDepositTransaction(String masterWalletID, String chainID, String fromAddress, String lockedAddress, long amount
            , String sideChainAddress, String memo,String remark,boolean useVotedUTXO, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createDepositTransaction( masterWalletID,  chainID,  fromAddress,  lockedAddress,  amount
            ,  sideChainAddress,  memo, remark, useVotedUTXO);
            }
        });
        subscriberObservable(observer, observable);
    }
    public void calculateTransactionFee(String walletId, String chainId, String data, long feePerKb, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonLongListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().calculateTransactionFee(walletId, chainId,data,feePerKb);
            }
        });
        subscriberObservable(observer, observable);
    }

    public void createWithdrawTransaction(String walletId, String chainId,String fromAddress, long actualSpend, String address,String memo   , String remark, BaseFragment baseFragment) {

        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createWithdrawTransaction(walletId, chainId,fromAddress, actualSpend, address, memo,remark);
            }
        });
        subscriberObservable(observer, observable);
    }
}
