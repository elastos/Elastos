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
