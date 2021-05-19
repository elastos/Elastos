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

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;

import java.util.HashMap;
import java.util.Map;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CtManagePresenter extends NewPresenterAbstract {

    public void getCRDepositcoin(String did, BaseFragment baseFragment) {
        Map<String, String> map = new HashMap();
        map.put("did", did);
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).getCRDepositcoin(map);
        Observer observer = createObserver(baseFragment, "getCRDepositcoin");
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getDIDByPublicKey(String masterWalletID, String publicKey, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getDIDByPublicKey");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getDIDByPublicKey(masterWalletID, publicKey);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getCRlist(int pageNum, int pageSize, String state, BaseFragment baseFragment,boolean isShow) {
        Map<String, Object> map = new HashMap();
        map.put("pageNum", pageNum);
        map.put("pageSize", pageSize);
        map.put("state", state);
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).getCRlist(map);
        Observer observer = createObserver(baseFragment, "getCRlist",isShow);
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getRegisteredCRInfo(String walletId, String chainID, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getRegisteredCRInfo",false);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getRegisteredCRInfo(walletId, chainID);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    //取回押金交易
    public void createRetrieveCRDepositTransaction(String masterWalletID, String chainID, String crPublickey, String amount, String memo, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "createRetrieveCRDepositTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createRetrieveCRDepositTransaction(masterWalletID, chainID, crPublickey, amount, memo);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

}
