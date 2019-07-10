package org.elastos.wallet.ela.ui.vote.NodeCart;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class NodeCartPresenter extends PresenterAbstract {


    //提交投票
    public void createVoteProducerTransaction(String masterWalletID, String chainID, String fromAddress, String stake, String publicKeys, String memo, boolean useVotedUTXO
            , BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createVoteProducerTransaction(masterWalletID, chainID, fromAddress, stake, publicKeys, memo, useVotedUTXO);
            }
        });
        subscriberObservable(observer, observable);
    }

}
