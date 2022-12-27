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

package org.elastos.wallet.ela.ui.Assets.presenter.mulwallet;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.CreateMasterWalletListner;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CreatMulWalletPresenter extends NewPresenterAbstract {

    public void exportxPrivateKey(String masterWalletID, String payPassword, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment,"exportxPrivateKey",payPassword);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().exportxPrivateKey(masterWalletID,
                        payPassword);
            }
        });
        subscriberObservable(observer, observable, baseFragment);

    }
    public void createMultiSignMasterWalletReadOnly(String masterWalletID, String coSigners, int requiredSignCount, boolean singleAddress, boolean compatible, long timestamp, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createMultiSignMasterWallet(masterWalletID, coSigners, requiredSignCount, singleAddress, compatible, timestamp);
            }
        });
        subscriberObservable(observer, observable, baseFragment);

    }


    public void createMultiSignMasterWalletByPrivKey(String masterWalletID, String privKey, String payPassword,
                                            String coSigners, int requiredSignCount, boolean singleAddress, boolean compatible, long timestamp, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createMultiSignMasterWallet(masterWalletID, privKey, payPassword,
                        coSigners, requiredSignCount, singleAddress, compatible, timestamp);
            }
        });
        subscriberObservable(observer, observable, baseFragment);

    }

    public void createMultiSignMasterWalletByMnemonic(String masterWalletId, String mnemonic, String phrasePassword, String payPassword,
                                            String coSigners, int requiredSignCount, boolean singleAddress, boolean compatible, long timestamp, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createMultiSignMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
                        coSigners, requiredSignCount, singleAddress, compatible, timestamp);
            }
        });
        subscriberObservable(observer, observable, baseFragment);

    }
}
