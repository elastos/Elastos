package org.elastos.wallet.ela.ui.did.presenter;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.utils.FileUtile;

import java.io.File;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class CredencialPresenter extends NewPresenterAbstract {

    public void keepFile(File filePath, String content, BaseFragment baseFragment) {
        Observer observer = createObserver(baseFragment, "keepFile");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return new CommmonBooleanEntity(MyWallet.SUCCESSCODE, FileUtile.writeFile(filePath, content));
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

}
