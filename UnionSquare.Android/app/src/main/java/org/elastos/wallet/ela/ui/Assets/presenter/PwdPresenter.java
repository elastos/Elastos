package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class PwdPresenter extends PresenterAbstract {
    //所有转账都要经历的方法  calculateTransactionFee 步骤1 2 3
//步骤1
    public void updateTransactionFee(String walletId, String chainID, String rawTransaction, long fee, String fromAddress, BaseActivity baseActivity) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseActivity);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getWallet().updateTransactionFee(walletId, chainID, rawTransaction, fee, fromAddress);
            }
        });
        subscriberObservable(observer, observable);
    }

    //步骤2
    public void signTransaction(String walletId, String chainId, String rawTransaction, String pwd, BaseActivity baseActivity) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseActivity);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getWallet().signTransaction(walletId, chainId, rawTransaction, pwd);
            }
        });
        subscriberObservable(observer, observable);
    }

    //步骤3
    public void publishTransaction(String walletId, String chainId, String rawTransaction, BaseActivity baseActivity) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseActivity);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getWallet().publishTransaction(walletId, chainId, rawTransaction);
            }
        });
        subscriberObservable(observer, observable);
    }


    //所有转账都要经历的方法  calculateTransactionFee 步骤1 2 3
//步骤1
    public void updateTransactionFee(String walletId, String chainID, String rawTransaction, long fee, String fromAddress, BaseFragment baseActivity) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseActivity);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().updateTransactionFee(walletId, chainID, rawTransaction, fee, fromAddress);
            }
        });
        subscriberObservable(observer, observable);
    }

    //步骤2
    public void signTransaction(String walletId, String chainId, String rawTransaction, String pwd, BaseFragment baseActivity) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseActivity);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().signTransaction(walletId, chainId, rawTransaction, pwd);
            }
        });
        subscriberObservable(observer, observable);
    }

    //步骤3
    public void publishTransaction(String walletId, String chainId, String rawTransaction, BaseFragment baseActivity) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseActivity);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().publishTransaction(walletId, chainId, rawTransaction);
            }
        });
        subscriberObservable(observer, observable);
    }


}
