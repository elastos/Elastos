package org.elastos.wallet.ela.ui.did.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.ui.Assets.listener.GetAllSubWalletsListner;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class AddDIDPresenter extends NewPresenterAbstract {


    public void getAllPublicKeys(String walletId, String chainID, int start, int count, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getAllPublicKeys");
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

    public void generateDIDInfoPayload(String masterWalletID, String inputJson, String passwd, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "generateDIDInfoPayload");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().generateDIDInfoPayload(masterWalletID, inputJson, passwd);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getAllSubWallets(String walletId, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getAllSubWallets");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllSubWallets(walletId);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

}
