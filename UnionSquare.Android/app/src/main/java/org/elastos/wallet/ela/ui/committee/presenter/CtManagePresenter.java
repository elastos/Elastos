package org.elastos.wallet.ela.ui.committee.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import java.util.HashMap;
import java.util.Map;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CtManagePresenter extends NewPresenterAbstract {

    public void getCRDepositcoin(String did, BaseFragment baseFragment) {
        Map<String, String> map = new HashMap();
        map.put("did", did);
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).getCRDepositcoin(map);
        Observer observer = createObserver(baseFragment, "getCRDepositcoin");
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getRegisteredCRInfo(String walletId, String chainID, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getRegisteredCRInfo",false);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getRegisteredCRInfo(walletId, chainID);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    //取回押金交易
    public void createRetrieveCRDepositTransaction(String masterWalletID, String chainID, String crPublickey, String amount, String memo, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "createRetrieveCRDepositTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createRetrieveCRDepositTransaction(masterWalletID, chainID, crPublickey, amount, memo);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

}
