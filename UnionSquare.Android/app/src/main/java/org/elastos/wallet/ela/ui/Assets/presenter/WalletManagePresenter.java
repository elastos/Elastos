package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.ui.Assets.listener.DestroyWalletListner;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class WalletManagePresenter extends NewPresenterAbstract {


    public void exportWalletWithMnemonic(String walletId, String pwd, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().exportWalletWithMnemonic(walletId, pwd);
            }
        });
        subscriberObservable(observer, observable);
    }

    public void destroyWallet(String walletId, BaseFragment baseFragment) {
        Observer observer = createObserver(DestroyWalletListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().destroyWallet(walletId);
            }
        });
        subscriberObservable(observer, observable);
    }

    public void exportReadonlyWallet(String walletId, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().exportReadonlyWallet(walletId);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getPubKeyInfo(String walletId, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getPubKeyInfo");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getPubKeyInfo(walletId);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getMasterWalletBasicInfo(String walletId, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getMasterWalletBasicInfo");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getMasterWalletBasicInfo(walletId);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void verifyPayPassword(String walletId, String payPasswd, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "verifyPayPassword");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().verifyPayPassword(walletId, payPasswd);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void verifyPrivateKey(String walletId, String mnemonic, String passphrase, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "verifyPrivateKey");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().verifyPrivateKey(walletId, mnemonic, passphrase);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void verifyPassPhrase(String walletId, String passphrase, String payPasswd, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "verifyPassPhrase");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().verifyPassPhrase(walletId, passphrase, payPasswd);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }


    public void DIDResolve(String didString, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "DIDResolve");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyDID().DIDResolve(didString);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void DIDResolveWithTip(String didString, BaseFragment baseFragment,String type) {
        Observer observer = createObserver(baseFragment, "DIDResolveWithTip",type);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyDID().DIDResolveWithTip(didString);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }public void DIDResolveWithTip(String didString, BaseActivity baseActivity, String type) {
        Observer observer = createObserver(baseActivity, "DIDResolveWithTip",type);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyDID().DIDResolveWithTip(didString);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }
}
