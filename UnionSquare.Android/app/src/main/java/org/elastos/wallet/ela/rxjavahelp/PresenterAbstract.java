package org.elastos.wallet.ela.rxjavahelp;

import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.widget.Toast;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.Log;

import io.reactivex.Observable;
import io.reactivex.ObservableEmitter;
import io.reactivex.ObservableOnSubscribe;
import io.reactivex.Observer;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.disposables.Disposable;
import io.reactivex.schedulers.Schedulers;


public class PresenterAbstract implements DialogInterface.OnCancelListener {
    protected String TAG = getClass().getSimpleName();
    protected Disposable mDisposable;

    protected Context context;

    @Deprecated
    protected void subscriberObservable(Observer subscriber,
                                        Observable observable) {
        observable.subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .unsubscribeOn(Schedulers.io())
                .subscribe(subscriber);
    }

    protected void subscriberObservable(Observer subscriber,
                                        Observable observable, BaseFragment baseFragment) {
        observable.compose(baseFragment.bindToLife()).subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .unsubscribeOn(Schedulers.io())
                .subscribe(subscriber);
    }

    protected void subscriberObservable(Observer subscriber,
                                        Observable observable, BaseActivity baseActivity) {
        observable.compose(baseActivity.bindToLife()).subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .unsubscribeOn(Schedulers.io())
                .subscribe(subscriber);
    }

    protected Observable createObservable(ObservableListener listener) {

        return Observable.create(new ObservableOnSubscribe<BaseEntity>() {
            @Override
            public void subscribe(ObservableEmitter<BaseEntity> emitter) throws Exception {
                emitter.onNext(listener.subscribe());
                emitter.onComplete();
            }
        });
    }


    protected Observer<BaseEntity> createObserver(Class<? extends SubscriberOnNextLisenner> listener, BaseFragment baseFragment) {
        //初始化参数

        return createObserver(listener, baseFragment, true, null);
    }

    protected Observer<BaseEntity> createObserver(Class<? extends SubscriberOnNextLisenner> listener, BaseActivity baseActivity) {
        //初始化参数
        return createObserver(listener, baseActivity, true, null);
    }

    protected Observer<BaseEntity> createObserver(Class<? extends SubscriberOnNextLisenner> listener, BaseFragment baseFragment, boolean isShowDialog) {
        return createObserver(listener, baseFragment, isShowDialog, null);
    }

    protected Observer<BaseEntity> createObserver(Class<? extends SubscriberOnNextLisenner> listener, BaseActivity baseActivity, boolean isShowDialog) {
        return createObserver(listener, baseActivity, isShowDialog, null);
    }

    protected Observer<BaseEntity> createObserver(Class<? extends SubscriberOnNextLisenner> listener, BaseFragment baseFragment, Object o) {
        return createObserver(listener, baseFragment, true, o);
    }

    protected Observer<BaseEntity> createObserver(Class<? extends SubscriberOnNextLisenner> listener, BaseActivity baseActivity, Object o) {
        return createObserver(listener, baseActivity, true, o);
    }

    protected Observer<BaseEntity> createObserver(Class<? extends SubscriberOnNextLisenner> listener, BaseFragment baseFragment, boolean isShowDialog, Object o) {
        //初始化参数
        this.context = baseFragment.getBaseActivity();
        Dialog dialog;
        if (isShowDialog) {
            dialog = initProgressDialog(context);
        } else {
            dialog = null;
        }
        SubscriberOnNextLisenner lisener = LisenerFactor.create(listener);
        lisener.setViewData((BaseViewData) baseFragment);
        if (o != null) {
            lisener.setObj(o);
        }
        //创建 Observer
        // Dialog finalDialog = dialog;
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
                if (MyWallet.SUCCESSCODE.equals(value.getCode()) || "0".equals(value.getCode())
                        ||MyWallet.errorCodeDoInMeathed.equals(value.getCode())){
                    lisener.onNextLisenner(value);
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

    protected Observer<BaseEntity> createObserver(Class<? extends SubscriberOnNextLisenner> listener, BaseActivity baseActivity, boolean isShowDialog, Object o) {
        //初始化参数
        this.context = baseActivity;
        Dialog dialog;
        if (isShowDialog) {
            dialog=initProgressDialog(context);
        }else {
            dialog=null;
        }
        SubscriberOnNextLisenner lisener = LisenerFactor.create(listener);
        lisener.setViewData((BaseViewData) baseActivity);
        if (o != null) {
            lisener.setObj(o);
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
                if (MyWallet.SUCCESSCODE.equals(value.getCode()) || "0".equals(value.getCode())
                        ||MyWallet.errorCodeDoInMeathed.equals(value.getCode())) {
                    lisener.onNextLisenner(value);
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


    protected void dismissProgessDialog(Dialog dialog) {
        if (dialog != null && dialog.isShowing()) {
            dialog.dismiss();
        }
    }

/*
    protected void dismissProgessDialog() {
        if (DialogUtil.getHttpialog() != null && DialogUtil.getHttpialog().isShowing()) {
            DialogUtil.getHttpialog().dismiss();
            DialogUtil.setHttpialogNull();
        }
    }
*/

    protected Dialog initProgressDialog(Context context) {
        Dialog dialog = new DialogUtil().getHttpDialog(context, "loading...");
        dialog.setOnCancelListener(this);
        dialog.show();
        //  dialog.dismiss();
      /*  if (!dialog.isShowing()) {
            dialog.show();
        }*/
        return dialog;
    }

    @Override
    public void onCancel(DialogInterface dialog) {
        //解除观察者和被观察者的绑定
        finish();
        if (mDisposable != null && !mDisposable.isDisposed()) {
            mDisposable.dispose();
        }
    }

    protected void finish() {
        if (context instanceof BaseActivity) {
            BaseActivity b = (BaseActivity) context;
            b.onError();
        }
    }

    protected static int getResourceId(Context context, String resourceName, String resourceType) {
        return context.getResources().getIdentifier(resourceName, resourceType,
                context.getPackageName());
    }

    protected void showTips(BaseEntity entity) {
        String msg;
        try {
            int id = getResourceId(context, "error_" + entity.getCode(), "string");
            msg = context.getString(id);
        } catch (Exception e) {
            msg = entity.getMsg();
        }


        Toast.makeText(context, msg, Toast.LENGTH_SHORT).show();
    }
}
