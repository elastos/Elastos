package org.elastos.wallet.ela.ui.proposal.presenter;

import android.text.TextUtils;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import java.util.HashMap;
import java.util.Map;

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

    public void calculateProposalHash(String walletId, String payload, BaseFragment baseFragment, String pwd) {


        Observer observer = createObserver(baseFragment, "calculateProposalHash", pwd);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().calculateProposalHash(walletId, payload);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getSuggestion(String id, BaseFragment baseFragment) {
        Observable observable = RetrofitManager.webApiCreate().getSuggestion(id);
        Observer observer = createObserver(baseFragment, "getSuggestion");
        subscriberObservable(observer, observable, baseFragment);
    }

    public void proposalSearch(int pageStart, int pageLimit, String status, String search, BaseFragment baseFragment) {
        Map<String, Object> map = new HashMap<>();
        map.put("page", pageStart);
        map.put("results", pageLimit);
        map.put("status", status);
        if (!TextUtils.isEmpty(search))
            map.put("search", search);
        Observable observable = RetrofitManager.webApiCreate().proposalSearch(map);
        Observer observer = createObserver(baseFragment, "proposalSearch");
        subscriberObservable(observer, observable, baseFragment);
    }
}
