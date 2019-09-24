package org.elastos.wallet.ela.rxjavahelp;

import android.util.Log;
import android.widget.Toast;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.base.BaseFragment;

import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;


public class NewPresenterAbstract extends PresenterAbstract {

    protected Observer<BaseEntity> createObserver(BaseFragment baseFragment, String methodName) {
        //初始化参数
        return createObserver(baseFragment, methodName, null);
    }

    protected Observer<BaseEntity> createObserver(BaseActivity baseActivity, String methodName) {
        //初始化参数
        return createObserver(baseActivity, methodName, null);
    }

    protected Observer<BaseEntity> createObserver(BaseFragment baseFragment, String methodName, boolean isShowDialog) {
        //初始化参数
        this.isShowDialog = isShowDialog;
        return createObserver(baseFragment, methodName, null);
    }

    protected Observer<BaseEntity> createObserver(BaseActivity baseActivity, String methodName, boolean isShowDialog) {
        //初始化参数
        this.isShowDialog = isShowDialog;
        return createObserver(baseActivity, methodName, null);
    }

    protected Observer<BaseEntity> createObserver(BaseFragment baseFragment, String methodName, Object o, boolean isShowDialog) {
        //初始化参数
        this.isShowDialog = isShowDialog;
        return createObserver(baseFragment, methodName, o);
    }

    protected Observer<BaseEntity> createObserver(BaseActivity baseActivity, String methodName, Object o, boolean isShowDialog) {
        //初始化参数
        this.isShowDialog = isShowDialog;
        return createObserver(baseActivity, methodName, o);
    }

    protected Observer<BaseEntity> createObserver(BaseFragment baseFragment, String methodName, Object o) {
        //初始化参数
        this.context = baseFragment.getBaseActivity();
        if (isShowDialog) {
            initProgressDialog(context);
        }
        //创建 Observer
        return new Observer<BaseEntity>() {
            @Override
            public void onSubscribe(Disposable d) {
                mDisposable = d;
                Log.e(TAG, "onSubscribe");
            }

            @Override
            public void onNext(BaseEntity value) {
                if (isShowDialog) {
                    dismissProgessDialog();
                }
                if (MyWallet.SUCCESSCODE.equals(value.getCode()) || "0".equals(value.getCode())) {
                    ((NewBaseViewData) baseFragment).onGetData(methodName, value, o);
                } else {
                    showTips(value);
                }
                Log.e(TAG, "onNext:" + value);

            }

            @Override
            public void onError(Throwable e) {
                if (isShowDialog) {
                    dismissProgessDialog();
                }
                Log.e(TAG, "onError=" + e.getMessage());
                Toast.makeText(context, e.getMessage(), Toast.LENGTH_SHORT).show();
                finish();
            }

            @Override
            public void onComplete() {
                Log.e(TAG, "onComplete()");
                finish();
            }
        };


    }


    protected Observer<BaseEntity> createObserver(BaseActivity baseActivity, String methodName, Object o) {
        //初始化参数
        this.context = baseActivity;
        if (isShowDialog) {
            initProgressDialog(context);
        }
        //创建 Observer
        return new Observer<BaseEntity>() {
            @Override
            public void onSubscribe(Disposable d) {
                mDisposable = d;
                Log.e(TAG, "onSubscribe");
            }

            @Override
            public void onNext(BaseEntity value) {
                if (isShowDialog) {
                    dismissProgessDialog();
                }
                if (MyWallet.SUCCESSCODE.equals(value.getCode())) {
                    ((NewBaseViewData) baseActivity).onGetData(methodName, value, o);
                } else {
                    showTips(value);

                }
                Log.e(TAG, "onNext:" + value);

            }

            @Override
            public void onError(Throwable e) {
                Log.e(TAG, "onError=" + e.getMessage());
                Toast.makeText(context, e.getMessage(), Toast.LENGTH_SHORT).show();
                if (isShowDialog) {
                    dismissProgessDialog();
                }
                finish();
            }

            @Override
            public void onComplete() {
                Log.e(TAG, "onComplete()");
                finish();
            }
        };


    }
}
