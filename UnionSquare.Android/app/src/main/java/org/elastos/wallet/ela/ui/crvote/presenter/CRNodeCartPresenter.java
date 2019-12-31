package org.elastos.wallet.ela.ui.crvote.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CRNodeCartPresenter extends NewPresenterAbstract {


    //提交投票
    public void createVoteCRTransaction(String masterWalletID, String chainID, String fromAddress, String votes, String memo
            , String unActiveData, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "createVoteCRTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createVoteCRTransaction(masterWalletID, chainID, fromAddress, votes, memo, unActiveData);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

}
