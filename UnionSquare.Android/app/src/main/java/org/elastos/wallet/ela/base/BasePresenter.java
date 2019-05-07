package org.elastos.wallet.ela.base;

import org.elastos.wallet.ela.ElaWallet.MyWallet;

/**
 * author: fangxiaogang
 * date: 2018/9/1
 */

public class BasePresenter<T extends BaseContract.Baseview> implements BaseContract.Basepresenter<T> {

    protected T mView;
    public MyWallet wallet = new MyWallet();

    @Override
    public void attachview(T view) {
        this.mView = view;
    }

    @Override
    public void destroyview() {
        if (mView != null) {
            mView = null;
        }
    }
}
