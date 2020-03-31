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

    public void getDID(String walletId, String chainID, int start, int count, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "getDID", walletId);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                BaseEntity baseEntity = baseFragment.getMyWallet().getAllPublicKeys(walletId, chainID, start, count);
                if (MyWallet.SUCCESSCODE.equals(baseEntity.getCode())) {
                    AllPkEntity allPkEntity = JSON.parseObject(((CommmonStringEntity) baseEntity).getData(), AllPkEntity.class);
                    return baseFragment.getMyWallet().getDIDByPublicKey(walletId, allPkEntity.getPublicKeys().get(0));
                } else {
                    return baseEntity;
                }
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

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
