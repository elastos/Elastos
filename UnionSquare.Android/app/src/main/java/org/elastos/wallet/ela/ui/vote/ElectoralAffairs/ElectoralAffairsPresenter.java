package org.elastos.wallet.ela.ui.vote.ElectoralAffairs;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class ElectoralAffairsPresenter extends PresenterAbstract {

    /*   //获取选举信息
       public void getRegisteredProducerInfo(String walletId, String chainID, BaseFragment baseFragment) {
           Observer observer = createObserver(RegisteredProducerInfoListner.class, baseFragment);
           Observable observable = createObservable(new ObservableListener() {
               @Override
               public BaseEntity subscribe() {
                   return baseFragment.getMyWallet().getRegisteredProducerInfo(walletId, chainID);
               }
           });
           subscriberObservable(observer, observable);
       }
   */
    //验证密码
    public void generateCancelProducerPayload(String masterWalletID, String chainID, String publicKey, String payPasswd, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().generateCancelProducerPayload(masterWalletID, chainID, publicKey, payPasswd);
            }
        });
        subscriberObservable(observer, observable);
    }

    //创建交易
    public void createCancelProducerTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, String memo, boolean useVotedUTXO, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createCancelProducerTransaction(masterWalletID, chainID, fromAddress, payloadJson, memo, useVotedUTXO);
            }
        });
        subscriberObservable(observer, observable);
    }


    //取回押金交易
    public void createRetrieveDepositTransaction(String masterWalletID, String chainID, String amount, String memo, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createRetrieveDepositTransaction(masterWalletID, chainID, amount, memo);
            }
        });
        subscriberObservable(observer, observable);
    }

    //获取公钥
    public void getPublicKeyForVote(String walletId, String chainID, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getPublicKeyForVote(walletId, chainID);
            }
        });
        subscriberObservable(observer, observable);
    }
}
