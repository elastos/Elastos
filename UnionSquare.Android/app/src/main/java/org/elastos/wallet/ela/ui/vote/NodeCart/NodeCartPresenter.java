package org.elastos.wallet.ela.ui.vote.NodeCart;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class NodeCartPresenter extends NewPresenterAbstract {


    //提交投票
    public void createVoteProducerTransaction(String masterWalletID, String chainID, String fromAddress, String stake, String publicKeys, String memo
            ,String invalidCandidates, BaseFragment baseFragment) {
        Observer observer = createObserver( baseFragment,"createVoteProducerTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createVoteProducerTransaction(masterWalletID, chainID, fromAddress, stake, publicKeys, memo,invalidCandidates);
            }
        });
        subscriberObservable(observer, observable,baseFragment);
    }

}
