package org.elastos.wallet.ela.ui.vote.signupfor;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;

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






}
