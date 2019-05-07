package org.elastos.wallet.ela.ui.vote.myVote;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class MyVotePresenter extends PresenterAbstract {


    public void getVotedProducerList(String masterWalletID, String chainID
            , BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getVotedProducerList(masterWalletID, chainID);
            }
        });
        subscriberObservable(observer, observable);
    }

}
