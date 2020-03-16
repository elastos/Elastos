package org.elastos.wallet.ela.ui.did.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;

import java.util.HashMap;
import java.util.Map;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class AuthorizationPresenter extends NewPresenterAbstract {
    public void postData(String url,String jwt, BaseFragment baseFragment) {
        Map<String,  Object> map = new HashMap();
        map.put("jwt", jwt);
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).postData(url,map);
        Observer observer = createObserver(baseFragment, "postData");
        subscriberObservable(observer, observable, baseFragment);
    }
    public void jwtSave(String did, String jwt,BaseFragment baseFragment) {
        Map<String, String> map = new HashMap();
        map.put("id", did);
        map.put("jwt", jwt);
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).jwtSave(map);
        Observer observer = createObserver(baseFragment, "jwtSave");
        subscriberObservable(observer, observable, baseFragment);
    }
}
