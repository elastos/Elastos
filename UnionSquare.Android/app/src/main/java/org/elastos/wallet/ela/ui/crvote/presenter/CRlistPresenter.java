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
        Observer observer = createObserver(baseFragment, "getRegisteredCRInfo",false);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getRegisteredCRInfo(walletId, chainID);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }




    public void getCRlist(int pageNum, int pageSize, String state, BaseFragment baseFragment,boolean isShow) {
        Map<String, Object> map = new HashMap();
        map.put("pageNum", pageNum);
        map.put("pageSize", pageSize);
        map.put("state", state);
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).getCRlist(map);
        Observer observer = createObserver(baseFragment, "getCRlist",isShow);
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getActiveCRlist(int pageNum, int pageSize, BaseFragment baseFragment) {
        Map<String, Object> map = new HashMap();
        map.put("pageNum", pageNum);
        map.put("pageSize", pageSize);
        map.put("state", "all");
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).getCRlist(map);
        Observer observer = createObserver(baseFragment, "getActiveCRlist");
        subscriberObservable(observer, observable, baseFragment);
    }
}
