package org.elastos.wallet.ela.ui.committee.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CtDetailPresenter extends NewPresenterAbstract {

    public void getCouncilInfo(BaseFragment baseFragment, String id, String did) {
        Observable observable = RetrofitManager.webApiCreate().getCouncilInfo(id, did);
        Observer observer = createObserver(baseFragment, "getCouncilInfo");
        subscriberObservable(observer, observable, baseFragment);
    }

}
