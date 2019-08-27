package org.elastos.wallet.ela.ui.vote.ElectoralAffairs;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;

import java.util.HashMap;
import java.util.Map;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class VoteListPresenter extends PresenterAbstract {


    public void votelistbean(String moreInfo, BaseFragment baseFragment) {
        // initProgressDialog(baseFragment.getContext());
        Map<String, String> map = new HashMap();
        map.put("moreInfo", moreInfo);
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).votelistbean(map);
        Observer observer = createObserver(VotelistbeanListener.class, baseFragment);
        subscriberObservable(observer, observable, baseFragment);
    }

}