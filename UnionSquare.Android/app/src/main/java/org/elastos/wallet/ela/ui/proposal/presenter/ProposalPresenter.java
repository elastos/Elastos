package org.elastos.wallet.ela.ui.proposal.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class ProposalPresenter extends NewPresenterAbstract {

    public void proposalOwnerDigest(String walletId, String payload, BaseFragment baseFragment) {


        Observer observer = createObserver(baseFragment, "proposalOwnerDigest", walletId);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().proposalOwnerDigest(walletId, payload);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void proposalCRCouncilMemberDigest(String walletId, String payload, BaseFragment baseFragment) {


        Observer observer = createObserver(baseFragment, "proposalCRCouncilMemberDigest", walletId);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().proposalCRCouncilMemberDigest(walletId, payload);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void createProposalTransaction(String walletId, String payload, BaseFragment baseFragment, String pwd) {


        Observer observer = createObserver(baseFragment, "createProposalTransaction", pwd);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createProposalTransaction(walletId, payload);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getSuggestion(String id, BaseFragment baseFragment) {
        Observable observable = RetrofitManager.webApiCreate().getSuggestion(id);
        Observer observer = createObserver(baseFragment, "getSuggestion");
        subscriberObservable(observer, observable, baseFragment);
    }
}
