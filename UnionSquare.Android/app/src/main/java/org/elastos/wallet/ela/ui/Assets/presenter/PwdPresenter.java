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

package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.ui.Assets.listener.SignTransactionListener;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class PwdPresenter extends NewPresenterAbstract {
    //所有转账都要经历的方法  步骤1 2 activitry
//步骤1
    public void signTransaction(String walletId, String chainId, String rawTransaction, String pwd, BaseActivity baseActivity) {
        Observer observer = createObserver(SignTransactionListener.class, baseActivity);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getWallet().signTransaction(walletId, chainId, rawTransaction, pwd);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }

    public void signDigest(String walletId, String cid, String digest, String pwd, BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "signDigest");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getWallet().signDigest(walletId, cid, digest, pwd);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }

    //步骤2
    public void publishTransaction(String walletId, String chainId, String rawTransaction, BaseActivity baseActivity) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseActivity);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getWallet().publishTransaction(walletId, chainId, rawTransaction);
            }
        });
        subscriberObservable(observer, observable);
    }


    //所有转账都要经历的方法   步骤1 2    fragment
//步骤1
    public void signTransaction(String walletId, String chainId, String rawTransaction, String pwd, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "signTransaction", pwd);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().signTransaction(walletId, chainId, rawTransaction, pwd);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void newPublishTransaction(String walletId, String chainId, String rawTransaction, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "newPublishTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().publishTransaction(walletId, chainId, rawTransaction);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    //步骤3
    public void publishTransaction(String walletId, String chainId, String rawTransaction, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().publishTransaction(walletId, chainId, rawTransaction);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    } //步骤3

    public void getTransactionSignedInfo(String walletId, String chainId, String rawTransaction, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getTransactionSignedInfo(walletId, chainId, rawTransaction);
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }


    //验证交易
    public void generateProducerPayload(String masterWalletID, String chainID, String publicKey, String nodePublicKey, String nickName, String url, String IPAddress, long location, String payPasswd
            , BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "generateProducerPayload");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getWallet().generateProducerPayload(masterWalletID, chainID, publicKey, nodePublicKey, nickName, url, IPAddress, location, payPasswd);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }

    //创建交易
    public void createUpdateProducerTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, String memo, boolean useVotedUTXO
            , BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "createUpdateProducerTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().createUpdateProducerTransaction(masterWalletID, chainID, fromAddress, payloadJson, memo, useVotedUTXO);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }

    //创建交易
    public void createRegisterProducerTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, String amount, String memo, boolean useVotedUTXO
            , BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "createRegisterProducerTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getWallet().createRegisterProducerTransaction(masterWalletID, chainID, fromAddress, payloadJson, amount, memo, useVotedUTXO);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }

    /************************************CR**************************************/
    //验证交易
    public void generateCRInfoPayload(String masterWalletID, String chainID, String publicKey, String nickName, String url, long location, String did
            , BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "generateCRInfoPayload");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().generateCRInfoPayload(masterWalletID, chainID, publicKey, nickName, url, location, did);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }


    //创建交易
    public void createRegisterCRTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, String amount, String memo, boolean useVotedUTXO
            , BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "createRegisterCRTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().createRegisterCRTransaction(masterWalletID, chainID, fromAddress, payloadJson, amount, memo, useVotedUTXO);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }


    //更新信息
    public void createUpdateCRTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, String memo, boolean useVotedUTXO
            , BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "createUpdateCRTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().createUpdateCRTransaction(masterWalletID, chainID, fromAddress, payloadJson, memo, useVotedUTXO);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }

    public void generateCancelProducerPayload(String masterWalletID, String chainID, String publicKey, String payPasswd, BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "generateCancelProducerPayload");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().generateCancelProducerPayload(masterWalletID, chainID, publicKey, payPasswd);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }


    //创建交易
    public void createCancelProducerTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "createCancelProducerTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().createCancelProducerTransaction(masterWalletID, chainID, fromAddress, payloadJson);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }

    //cr
    public void generateUnregisterCRPayload(String masterWalletID, String chainID, String cid, BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "generateUnregisterCRPayload");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().generateUnregisterCRPayload(masterWalletID, chainID, cid);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }


    //注销cr
    public void createUnregisterCRTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "createUnregisterCRTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().createUnregisterCRTransaction(masterWalletID, chainID, fromAddress, payloadJson);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }

    //注册did
    public void generateDIDInfoPayload(String masterWalletID, String inputJson, String passwd, BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "generateDIDInfoPayload");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().generateDIDInfoPayload(masterWalletID, inputJson, passwd);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }

    public void createIDTransaction(String masterWalletID, String inputJson, BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "createIDTransaction");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().createIDTransaction(masterWalletID, inputJson);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }

    public void DIDPublish(String pwd, BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "DIDPublish", false);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyDID().DIDPublish(pwd);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }
}
