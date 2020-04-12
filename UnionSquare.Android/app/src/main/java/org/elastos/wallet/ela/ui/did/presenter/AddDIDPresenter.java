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

package org.elastos.wallet.ela.ui.did.presenter;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.did.entity.AllPkEntity;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class AddDIDPresenter extends NewPresenterAbstract {


    public void getAllPublicKeys(String walletId, String chainID, int start, int count, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getAllPublicKeys", walletId);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllPublicKeys(walletId, chainID, start, count);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }


    public void getCIDByPublicKey(String masterWalletID, String publicKey, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getCIDByPublicKey");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getCIDByPublicKey(masterWalletID, publicKey);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }


    public void getAllSubWallets(String walletId, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getAllSubWallets", walletId);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllSubWallets(walletId);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    /**
     * 为了逻辑清晰使用不同方法名处理不同位置调用后的结果
     *
     * @param walletId
     * @param baseFragment
     */
    public void getAllSubWallets1(String walletId, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getAllSubWallets1", walletId);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getAllSubWallets(walletId);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }
}
