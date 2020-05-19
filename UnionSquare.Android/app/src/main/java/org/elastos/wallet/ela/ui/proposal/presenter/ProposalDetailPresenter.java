package org.elastos.wallet.ela.ui.proposal.presenter;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;

import java.util.ArrayList;
import java.util.List;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class ProposalDetailPresenter extends NewPresenterAbstract {
    public void proposalDetail(int id, BaseFragment baseFragment) {

        Observable observable = RetrofitManager.webApiCreate().getProposalDetail(id);
        Observer observer = createObserver(baseFragment, "proposalDetail");
        subscriberObservable(observer, observable, baseFragment);
    }

    public void createVoteCRCProposalTransaction(String walletId, String votes, String invalidCandidates, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "createVoteCRCProposalTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createVoteCRCProposalTransaction(walletId, votes, invalidCandidates);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getVoteInfo(String walletId, String type, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getVoteInfo");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getVoteInfo(walletId, type);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }


    public void proposalTrackingOwnerDigest(String walletId, String payload, BaseFragment baseFragment, String pwd) {
        Observer observer = createObserver(baseFragment, "proposalTrackingOwnerDigest", pwd);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().proposalTrackingOwnerDigest(walletId, payload);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void proposalTrackingNewOwnerDigest(String walletId, String payload, BaseFragment baseFragment, String pwd) {
        Observer observer = createObserver(baseFragment, "proposalTrackingNewOwnerDigest", pwd);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().proposalTrackingNewOwnerDigest(walletId, payload);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void proposalTrackingSecretaryDigest(String walletId, String payload, BaseFragment baseFragment, String pwd) {
        Observer observer = createObserver(baseFragment, "ProposalTrackingSecretaryDigest", pwd);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().proposalTrackingSecretaryDigest(walletId, payload);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public JSONObject getCRUnactiveData(List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list) {

        List<String> candidates = new ArrayList<>();
        if (list != null && list.size() > 0) {
            for (int i = 0; i < list.size(); i++) {
                CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean = list.get(i);
                if (!bean.getState().equals("Active")) {
                    candidates.add(bean.getDid());
                }
            }
        }
        return getActiveJson("CRC", candidates);
    }

    public JSONObject getDepositUnactiveData(List<VoteListBean.DataBean.ResultBean.ProducersBean> list) {

        List<String> candidates = new ArrayList<>();
        if (list != null && list.size() > 0) {
            for (int i = 0; i < list.size(); i++) {
                VoteListBean.DataBean.ResultBean.ProducersBean bean = list.get(i);
                if (!bean.getState().equals("Active")) {
                    candidates.add(bean.getOwnerpublickey());
                }
            }
        }
        return getActiveJson("Delegate", candidates);
    }

    private JSONObject getActiveJson(String type, List<String> list) {
        JSONObject unActiveVote = new JSONObject();
        unActiveVote.put("Type", type);
        unActiveVote.put("Candidates", JSON.toJSON(list));
        return unActiveVote;
    }
}
