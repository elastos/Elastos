package org.elastos.wallet.ela.ui.crvote.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;
import org.elastos.wallet.ela.ui.common.listener.ServerCommonObjectWithMNListener;

import java.util.HashMap;
import java.util.Map;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CRManagePresenter extends NewPresenterAbstract {
    public void getCRDepositcoin(String ownerPublicKey, BaseFragment baseFragment) {
        Map<String, String> map = new HashMap();
        map.put("ownerpublickey", ownerPublicKey);
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).getCRDepositcoin(map);
        Observer observer = createObserver( baseFragment, "getCRDepositcoin");
        subscriberObservable(observer, observable, baseFragment);
    }
}
