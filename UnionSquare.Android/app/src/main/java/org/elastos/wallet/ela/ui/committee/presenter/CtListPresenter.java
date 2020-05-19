package org.elastos.wallet.ela.ui.committee.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CtListPresenter extends NewPresenterAbstract {

    public void getCouncilList(BaseFragment baseFragment, String id) {
        Observable observable = RetrofitManager.webApiCreate().getCouncilList(id);
        Observer observer = createObserver(baseFragment, "getCouncilList");
        subscriberObservable(observer, observable, baseFragment);
    }

    public void createImpeachmentCRCTransaction(String masterWalletID, String chainID, String fromAddress, String votes, String memo
            , String unActiveData, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "createImpeachmentCRCTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createImpeachmentCRCTransaction(masterWalletID, chainID, fromAddress, votes, memo, unActiveData);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getVoteInfo(String masterWalletID, String chainID, String type, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getVoteInfo");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getVoteInfo(masterWalletID, chainID, type);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

}
