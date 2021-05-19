/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.wallet.ela.ui.committee.presenter;

import android.text.TextUtils;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalSearchEntity;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CtDetailPresenter extends NewPresenterAbstract {

    public void getCouncilInfo(BaseFragment baseFragment, String id, String did) {
        Observable observable = RetrofitManager.webApiCreate().getCouncilInfo(id, did);
        Observer observer = createObserver(baseFragment, "getCouncilInfo");
        subscriberObservable(observer, observable, baseFragment, 3);
    }

    public void getCurrentCouncilInfo(BaseFragment baseFragment, String did, String type) {
        Observable observable = RetrofitManager.webApiCreate().getCurrentCouncilInfo(did).retry();
        Observer observer = createObserver(baseFragment, "getCurrentCouncilInfo", type);
        subscriberObservable(observer, observable, baseFragment, 3);
    }

    public void getCurrentCouncilInfo(BaseFragment baseFragment, String did) {
        Observable observable = RetrofitManager.webApiCreate().getCurrentCouncilInfo(did).retry();
        Observer observer = createObserver(baseFragment, "getCurrentCouncilInfo");
        subscriberObservable(observer, observable, baseFragment, 3);
    }

    public void getVoteInfo(String masterWalletID, String type, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getVoteInfo", type);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getVoteInfo(masterWalletID, type);
            }
        });
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
        subscriberObservable(observer, observable, baseFragment, 3);
    }

    public JSONObject getPublishDataFromLastVote(JSONObject lastVote, String amount, ArrayList<ProposalSearchEntity.DataBean.ListBean> searchBeanList) {
        JSONObject newVotes = new JSONObject();
        try {
            Iterator it = lastVote.keys();
            while (it.hasNext()) {
                String key = (String) it.next();
                for (int i = 0; i < searchBeanList.size(); i++) {
                    if (searchBeanList.get(i).getProposalHash().equals(key)) {
                        newVotes.put(key, amount);
                        break;
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return newVotes;
    }

    public JSONObject getCRUnactiveData(List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list) {

        JSONArray candidates = new JSONArray();
        if (list != null && list.size() > 0) {
            for (int i = 0; i < list.size(); i++) {
                CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean = list.get(i);
                if (!bean.getState().equals("Active")) {
                    candidates.put(bean.getDid());
                }
            }
        }
        return getActiveJson("CRC", candidates);
    }


    public JSONObject getProposalUnactiveData(List<ProposalSearchEntity.DataBean.ListBean> list) {
        JSONArray candidates = new JSONArray();
        if (list != null && list.size() > 0) {
            for (int i = 0; i < list.size(); i++) {
                ProposalSearchEntity.DataBean.ListBean bean = list.get(i);
                if (!bean.getStatus().equalsIgnoreCase("NOTIFICATION")) {
                    String hash = bean.getProposalHash();
                    if(!AppUtlis.isNullOrEmpty(hash)) candidates.put(hash);
                }
            }
        }
        return getActiveJson("CRCProposal", candidates);
    }

    public JSONObject conversVote(String voteInfo) {
        if (!TextUtils.isEmpty(voteInfo) && !voteInfo.equals("null") && !voteInfo.equals("[]")) {
            try {
                JSONArray lastVoteInfo = new JSONArray(voteInfo);
                if (lastVoteInfo.length() >= 1) {
                    return lastVoteInfo.getJSONObject(0).getJSONObject("Votes");
                }
            } catch (JSONException e) {
                e.printStackTrace();
            }

        }
        return null;
    }

    public JSONObject getCRLastVote(JSONObject origin) {
        return getActiveJson("CRC", getJsonKeys(origin));
    }

    public JSONObject getProposalLastVote(JSONObject origin) {
        return getActiveJson("CRCProposal", getJsonKeys(origin));
    }

    public JSONObject getImpeachmentVote(JSONObject origin) {
        return getActiveJson("CRCImpeachment", getJsonKeys(origin));
    }

    public JSONArray getJsonKeys(JSONObject origin) {
        JSONArray target = new JSONArray();
        if (origin != null) {
            Iterator it = origin.keys();
            while (it.hasNext()) {
                target.put(it.next());

            }
        }
        return target;
    }

    public JSONObject getDepositUnactiveData(List<VoteListBean.DataBean.ResultBean.ProducersBean> list) {

        JSONArray candidates = new JSONArray();
        if (list != null && list.size() > 0) {
            for (int i = 0; i < list.size(); i++) {
                VoteListBean.DataBean.ResultBean.ProducersBean bean = list.get(i);
                if (!bean.getState().equals("Active")) {
                    candidates.put(bean.getOwnerpublickey());
                }
            }
        }
        return getActiveJson("Delegate", candidates);
    }

    public JSONObject getActiveJson(String type, JSONArray jsonArray) {
        JSONObject unActiveVote = new JSONObject();

        try {
            unActiveVote.put("Type", type);
            unActiveVote.put("Candidates", jsonArray);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return unActiveVote;
    }
}
