package org.elastos.wallet.ela.ui.crvote.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;
import org.elastos.wallet.ela.ui.common.listener.ServerCommonObjectWithMNListener;

import java.util.HashMap;
import java.util.Map;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CRManagePresenter extends NewPresenterAbstract {


    public void generateUnregisterCRPayload(String masterWalletID, String chainID, String publicKey, String payPasswd, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "generateUnregisterCRPayload");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().generateUnregisterCRPayload(masterWalletID, chainID, publicKey, payPasswd);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    //创建交易
    public void createUnregisterCRTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, String memo, boolean useVotedUTXO, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "createUnregisterCRTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createUnregisterCRTransaction(masterWalletID, chainID, fromAddress, payloadJson, memo, useVotedUTXO);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }


    //取回押金交易
    public void createRetrieveCRDepositTransaction(String masterWalletID, String chainID, String amount, String memo, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "createRetrieveCRDepositTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createRetrieveCRDepositTransaction(masterWalletID, chainID, amount, memo);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }


    public void getCRDepositcoin(String ownerPublicKey, BaseFragment baseFragment) {
        Map<String, String> map = new HashMap();
        map.put("ownerpublickey", ownerPublicKey);
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).getCRDepositcoin(map);
        Observer observer = createObserver( baseFragment, "getCRDepositcoin");
        subscriberObservable(observer, observable, baseFragment);
    }
}
