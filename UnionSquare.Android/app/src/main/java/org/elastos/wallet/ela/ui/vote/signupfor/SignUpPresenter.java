package org.elastos.wallet.ela.ui.vote.signupfor;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;
import org.json.JSONException;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class SignUpPresenter extends PresenterAbstract {

    //获取ownerpublickey公钥
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


    //验证交易
    public void getGenerateProducerPayload(String masterWalletID, String chainID, String publicKey, String nodePublicKey, String nickName, String url, String IPAddress, long location, String payPasswd
            , BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() throws JSONException {
                return baseFragment.getMyWallet().generateProducerPayload(masterWalletID, chainID, publicKey, nodePublicKey, nickName, url, IPAddress, location, payPasswd);
            }
        });
        subscriberObservable(observer, observable);
    }


    //创建交易
    public void createRegisterProducerTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, String amount, String memo, boolean useVotedUTXO
            , BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createRegisterProducerTransaction(masterWalletID, chainID, fromAddress, payloadJson, amount, memo, useVotedUTXO);
            }
        });
        subscriberObservable(observer, observable);
    }


    //更新信息

    //创建交易
    public void createUpdateProducerTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, String memo, boolean useVotedUTXO
            , BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createUpdateProducerTransaction(masterWalletID, chainID, fromAddress, payloadJson, memo, useVotedUTXO);
            }
        });
        subscriberObservable(observer, observable);
    }
}
