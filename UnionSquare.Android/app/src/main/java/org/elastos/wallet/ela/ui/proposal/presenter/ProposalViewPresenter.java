package org.elastos.wallet.ela.ui.proposal.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class ProposalViewPresenter extends NewPresenterAbstract {
    public void proposalDetail(int id, BaseFragment baseFragment) {

        Observable observable = RetrofitManager.webApiCreate().getProposalDetail(id);
        Observer observer = createObserver(baseFragment, "proposalDetail");
        subscriberObservable(observer, observable, baseFragment);
    }
}
