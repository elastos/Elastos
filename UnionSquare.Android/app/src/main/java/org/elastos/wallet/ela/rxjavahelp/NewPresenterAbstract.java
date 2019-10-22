package org.elastos.wallet.ela.rxjavahelp;

import android.app.Dialog;
import org.elastos.wallet.ela.utils.Log;
import android.widget.Toast;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.base.BaseFragment;

import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;


public class NewPresenterAbstract extends PresenterAbstract {

    protected Observer<BaseEntity> createObserver(BaseFragment baseFragment, String methodName) {
        //初始化参数
        return createObserver(baseFragment, methodName, true, null);
    }

    protected Observer<BaseEntity> createObserver(BaseActivity baseActivity, String methodName) {
        //初始化参数
        return createObserver(baseActivity, methodName, true, null);
    }

    protected Observer<BaseEntity> createObserver(BaseFragment baseFragment, String methodName, boolean isShowDialog) {
        //初始化参数

        return createObserver(baseFragment, methodName, isShowDialog, null);
    }

    protected Observer<BaseEntity> createObserver(BaseActivity baseActivity, String methodName, boolean isShowDialog) {
        //初始化参数
        return createObserver(baseActivity, methodName, isShowDialog, null);
    }

    protected Observer<BaseEntity> createObserver(BaseFragment baseFragment, String methodName, Object o) {
        //初始化参数
        return createObserver(baseFragment, methodName, true, o);
    }

    protected Observer<BaseEntity> createObserver(BaseActivity baseActivity, String methodName, Object o) {
        //初始化参数

        return createObserver(baseActivity, methodName, true, o);
    }

    protected Observer<BaseEntity> createObserver(BaseFragment baseFragment, String methodName, boolean isShowDialog, Object o) {
        //初始化参数
        this.context = baseFragment.getBaseActivity();
        Dialog dialog;
        if (isShowDialog) {
            dialog = initProgressDialog(context);
        } else {
            dialog = null;
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
                    dismissProgessDialog(dialog);
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
                    dismissProgessDialog(dialog);
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


    protected Observer<BaseEntity> createObserver(BaseActivity baseActivity, String methodName, boolean isShowDialog, Object o) {
        //初始化参数
        this.context = baseActivity;
        Dialog dialog;
        if (isShowDialog) {
            dialog = initProgressDialog(context);
        } else {
            dialog = null;
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
                    dismissProgessDialog(dialog);
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
                    dismissProgessDialog(dialog);
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
