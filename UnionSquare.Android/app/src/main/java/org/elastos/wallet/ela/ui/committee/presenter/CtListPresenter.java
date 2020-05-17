package org.elastos.wallet.ela.ui.committee.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CtListPresenter extends NewPresenterAbstract {

    public void getCouncilList(BaseFragment baseFragment, String id) {
        Observable observable = RetrofitManager.webApiCreate().getCouncilList(id);
        Observer observer = createObserver(baseFragment, "getCouncilList");
        subscriberObservable(observer, observable, baseFragment);
    }


}
