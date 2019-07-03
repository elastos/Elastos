package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.CreateMasterWalletListner;
import org.elastos.wallet.ela.ui.Assets.listener.GenerateMnemonicListner;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CreateMasterWalletPresenter extends PresenterAbstract {
    public void createMasterWallet(String masterWalletID, String mnemonic, String phrasePassword,
                                   String payPassword, boolean singleAddress,
                                   BaseFragment baseFragment) {
        Observer observer = createObserver(CreateMasterWalletListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createMasterWallet(masterWalletID, mnemonic, phrasePassword,
                        payPassword, singleAddress);
            }
        });
        subscriberObservable(observer, observable);

    }

    public void importReadonlyWallet(String masterWalletID, String walletJson, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().importReadonlyWallet(masterWalletID, walletJson);
            }
        });
        subscriberObservable(observer, observable,baseFragment);

    }

}
