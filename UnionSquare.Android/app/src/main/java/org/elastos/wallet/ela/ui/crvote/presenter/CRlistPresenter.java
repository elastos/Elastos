package org.elastos.wallet.ela.ui.crvote.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import java.util.HashMap;
import java.util.Map;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CRlistPresenter extends NewPresenterAbstract {


    public void getRegisteredCRInfo(String walletId, String chainID, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getRegisteredCRInfo");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getRegisteredCRInfo(walletId, chainID);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }


    //获取ownerpublickey公钥
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

    public void getCrlist(String moreInfo, BaseFragment baseFragment) {
        Map<String, String> map = new HashMap();
        map.put("moreInfo", moreInfo);
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).votelistbean(map);
        Observer observer = createObserver(baseFragment, "getCrlist");
        subscriberObservable(observer, observable, baseFragment);
    }
}
