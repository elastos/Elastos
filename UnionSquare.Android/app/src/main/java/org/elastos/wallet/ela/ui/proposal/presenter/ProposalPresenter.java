package org.elastos.wallet.ela.ui.proposal.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
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


}
