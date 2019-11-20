package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.utils.PingUtil;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class IPPresenter extends NewPresenterAbstract {


    public void setFixedPeer(String walletId, String chainId, String address, int port, BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "setFixedPeer",address);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseActivity.getMyWallet().setFixedPeer(walletId, chainId, address, port);
            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }

    public void ping(String defaultAdd, BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "ping", defaultAdd);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                boolean result = PingUtil.ping(defaultAdd);
                return new CommmonBooleanEntity(MyWallet.SUCCESSCODE, result);

            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }

}
