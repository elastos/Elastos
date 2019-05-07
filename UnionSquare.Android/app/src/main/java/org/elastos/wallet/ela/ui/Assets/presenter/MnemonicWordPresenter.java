package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.CreateMasterWalletListner;
import org.elastos.wallet.ela.ui.Assets.listener.CreateSubWalletListner;
import org.elastos.wallet.ela.ui.Assets.listener.GenerateMnemonicListner;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class MnemonicWordPresenter extends PresenterAbstract {
    public void generateMnemonic(String type, BaseFragment baseFragment) {
        Observer observer = createObserver(GenerateMnemonicListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().generateMnemonic(type);
            }
        });
        subscriberObservable(observer, observable);

    }

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


}
