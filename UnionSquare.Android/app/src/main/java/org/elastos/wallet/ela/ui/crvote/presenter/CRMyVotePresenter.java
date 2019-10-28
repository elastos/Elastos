package org.elastos.wallet.ela.ui.crvote.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CRMyVotePresenter extends NewPresenterAbstract {


    public void getVotedCRList(String masterWalletID, String chainID
            , BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getVotedCRList");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getVotedCRList(masterWalletID, chainID);
            }
        });
        subscriberObservable(observer, observable,baseFragment);
    }

}
