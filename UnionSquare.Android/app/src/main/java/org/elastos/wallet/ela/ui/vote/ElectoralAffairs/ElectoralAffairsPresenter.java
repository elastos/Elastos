package org.elastos.wallet.ela.ui.vote.ElectoralAffairs;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import java.util.HashMap;
import java.util.Map;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class ElectoralAffairsPresenter extends NewPresenterAbstract {

    //获取公钥
    public void getPublicKeyForVote(String walletId, String chainID, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getPublicKeyForVote");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getPublicKeyForVote(walletId, chainID);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    } //获取公钥

    public void getDepositcoin(String ownerPublicKey, BaseFragment baseFragment) {
        Map<String, String> map = new HashMap();
        map.put("ownerpublickey", ownerPublicKey);
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).getdepositcoin(map);
        Observer observer = createObserver(baseFragment, "getDepositcoin");
        subscriberObservable(observer, observable, baseFragment);
    }

    //取回押金交易
    public void createRetrieveDepositTransaction(String masterWalletID, String chainID, String amount, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "createRetrieveDepositTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createRetrieveDepositTransaction(masterWalletID, chainID, amount);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }
}
