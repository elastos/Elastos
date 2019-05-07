package org.elastos.wallet.ela.ui.main.presenter;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.bean.CommmonObjEntity;
import org.elastos.wallet.ela.ui.main.listener.MyWalletListener;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class MainPresenter extends PresenterAbstract {

    public void getWallet(BaseActivity baseActivity) {
        Observer observer = createObserver(MyWalletListener.class, baseActivity, false);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                MyWallet myWallet = baseActivity.getWallet();
                return new CommmonObjEntity(MyWallet.SUCESSCODE, myWallet);
            }
        });
        subscriberObservable(observer, observable);
    }


}
