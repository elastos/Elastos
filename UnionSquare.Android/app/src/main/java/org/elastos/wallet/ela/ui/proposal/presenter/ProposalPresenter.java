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

package org.elastos.wallet.ela.ui.proposal.presenter;

import android.content.Intent;
import android.os.Parcelable;
import android.text.TextUtils;
import android.util.Base64;

import com.alibaba.fastjson.JSON;

import org.elastos.did.exception.DIDStoreException;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecieveProposalFatherJwtEntity;
import org.elastos.wallet.ela.ui.did.presenter.AuthorizationPresenter;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalCallBackEntity;
import org.elastos.wallet.ela.ui.vote.activity.VertifyPwdActivity;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.utils.JwtUtils;
import org.elastos.wallet.ela.utils.Log;

import java.util.Date;
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

    public void proposalReviewDigest(String walletId, String payload, BaseFragment baseFragment, Object pwd) {
        Observer observer = createObserver(baseFragment, "proposalReviewDigest", pwd);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().proposalReviewDigest(walletId, payload);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void createProposalReviewTransaction(String walletId, String payload, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "createProposalReviewTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createProposalReviewTransaction(walletId, payload);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void showFeePage(Wallet wallet, String type, int transType, BaseFragment baseFragment, Parcelable extra) {
        //发送提案交易
        Intent intent = new Intent(baseFragment.getActivity(), VoteTransferActivity.class);
        intent.putExtra("wallet", wallet);
        intent.putExtra("chainId", MyWallet.ELA);
        intent.putExtra("fee", 10000L);
        intent.putExtra("type", type);
        intent.putExtra("extra", extra);
        intent.putExtra("transType", transType);
        intent.putExtra("openType", baseFragment.getClass().getSimpleName());
        baseFragment.startActivity(intent);
    }

    public String getSignDigist(String payPasswd, String digist, BaseFragment baseFragment) {

        try {
            String sign = baseFragment.getMyDID().getDIDDocument().signDigest(payPasswd, JwtUtils.hex2byteBe(digist));
            Log.i("signDigest", sign);
            return JwtUtils.bytesToHexString(Base64.decode(sign, Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP));

        } catch (DIDStoreException e) {
            e.printStackTrace();
        }
        return null;

    }

    public void toVertifyPwdActivity(Wallet wallet, BaseFragment baseFragment) {
        Intent intent = new Intent(baseFragment.getActivity(), VertifyPwdActivity.class);
        intent.putExtra("walletId", wallet.getWalletId());
        intent.putExtra("openType", baseFragment.getClass().getSimpleName());
        baseFragment.startActivity(intent);
    }

    public void backProposalJwt(String type, String scanResult, String data, String payPasswd, BaseFragment baseFragment) {
        String result = scanResult.replace("elastos://crproposal/", "");
        RecieveProposalFatherJwtEntity entity = JSON.parseObject(JwtUtils.getJwtPayload(result), RecieveProposalFatherJwtEntity.class);

        ProposalCallBackEntity callBackJwtEntity = new ProposalCallBackEntity();
        callBackJwtEntity.setType(type);
        callBackJwtEntity.setIss(baseFragment.getMyDID().getDidString());
        callBackJwtEntity.setIat(new Date().getTime() / 1000);
        callBackJwtEntity.setExp(new Date().getTime() / 1000 + 10 * 60);
        callBackJwtEntity.setAud(entity.getIss());
        callBackJwtEntity.setReq(scanResult);
        callBackJwtEntity.setCommand(entity.getCommand());
        callBackJwtEntity.setData(data);
        String header = JwtUtils.getJwtHeader(result);
        // Base64
        String payload = Base64.encodeToString(JSON.toJSONString(callBackJwtEntity).getBytes(), Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
        payload = payload.replaceAll("=", "");
        try {
            String signature = baseFragment.getMyDID().getDIDDocument().sign(payPasswd, (header + "." + payload).getBytes());
            new AuthorizationPresenter().postData(entity.getCallbackurl(), header + "." + payload + "." + signature, baseFragment);
        } catch (DIDStoreException e) {
            e.printStackTrace();
        }
    }

    public void proposalTrackingOwnerDigest(String walletId, String payload, BaseFragment baseFragment, Object o) {
        Observer observer = createObserver(baseFragment, "proposalTrackingOwnerDigest", o);
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

    public void proposalTrackingSecretaryDigest(String walletId, String payload, BaseFragment baseFragment, Object o) {
        Observer observer = createObserver(baseFragment, "proposalTrackingSecretaryDigest", o);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().proposalTrackingSecretaryDigest(walletId, payload);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void createProposalTrackingTransaction(String walletId, String payload, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "createProposalTrackingTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createProposalTrackingTransaction(walletId, payload);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void proposalWithdrawDigest(String walletId, String payload, BaseFragment baseFragment, Object o) {
        Observer observer = createObserver(baseFragment, "proposalWithdrawDigest", o);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().proposalWithdrawDigest(walletId, payload);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void createProposalWithdrawTransaction(String walletId, String recipient, String amount, String utxo, String payload, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "createProposalWithdrawTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createProposalWithdrawTransaction(walletId, recipient, amount, utxo, payload);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getCurrentCouncilInfo(String did, BaseFragment baseFragment) {
        Observable observable = RetrofitManager.webApiCreate().getCurrentCouncilInfo(did);
        Observer observer = createObserver(baseFragment, "getCurrentCouncilInfo");
        subscriberObservable(observer, observable, baseFragment);
    }
}
