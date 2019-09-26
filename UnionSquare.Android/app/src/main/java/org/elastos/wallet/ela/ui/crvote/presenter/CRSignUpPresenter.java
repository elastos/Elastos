package org.elastos.wallet.ela.ui.crvote.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CRSignUpPresenter extends NewPresenterAbstract {


    public void getCROwnerPublicKey(String walletId, String chainID, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getCROwnerPublicKey");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getCROwnerPublicKey(walletId, chainID);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    //验证交易
    public void generateCRInfoPayload(String masterWalletID, String chainID, String publicKey, String nickName, String url, long location, String payPasswd
            , BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "generateCRInfoPayload");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().generateCRInfoPayload(masterWalletID, chainID, publicKey, nickName, url, location, payPasswd);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }


    //更新信息
    public void createUpdateCRTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, String memo, boolean useVotedUTXO
            , BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "createUpdateCRTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createUpdateCRTransaction(masterWalletID, chainID, fromAddress, payloadJson, memo, useVotedUTXO);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getFee(String walletId, String chainId, String s, String address, String amount, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getFee");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getFee(walletId, chainId, s, address, amount);
            }
        });
        subscriberObservable(observer, observable,baseFragment);
    }
}
