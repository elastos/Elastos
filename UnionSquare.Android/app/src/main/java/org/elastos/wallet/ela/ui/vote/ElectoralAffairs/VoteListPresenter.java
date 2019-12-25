package org.elastos.wallet.ela.ui.vote.ElectoralAffairs;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;

import java.util.HashMap;
import java.util.Map;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class VoteListPresenter extends NewPresenterAbstract {



    public void getDepositVoteList(String moreInfo, String state, BaseFragment baseFragment, boolean isShow) {
        // initProgressDialog(baseFragment.getContext());
        Map<String, String> map = new HashMap();
        map.put("moreInfo", moreInfo);
        map.put("state", state);
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).votelistbean(map);
        Observer observer = createObserver(baseFragment, "getDepositVoteList", isShow);
        subscriberObservable(observer, observable, baseFragment);
    }

}