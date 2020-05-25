package org.elastos.wallet.ela.ui.committee.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class PastCtPresenter extends NewPresenterAbstract {

    public void getCouncilTerm(BaseFragment baseFragment) {
        Observable observable = RetrofitManager.webApiCreate().getCouncilTerm();
        Observer observer = createObserver(baseFragment, "getCouncilTerm");
        subscriberObservable(observer, observable, baseFragment, true);
    }



}
