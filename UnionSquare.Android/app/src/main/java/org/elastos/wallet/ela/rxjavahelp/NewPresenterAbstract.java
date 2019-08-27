package org.elastos.wallet.ela.rxjavahelp;

import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.util.Log;
import android.widget.Toast;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.DialogUtil;

import io.reactivex.Observable;
import io.reactivex.ObservableEmitter;
import io.reactivex.ObservableOnSubscribe;
import io.reactivex.Observer;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.disposables.Disposable;
import io.reactivex.schedulers.Schedulers;


public class NewPresenterAbstract implements DialogInterface.OnCancelListener {
    private String TAG = getClass().getSimpleName();
    private Disposable mDisposable;
    private boolean isShowDialog = true;
    private Context context;


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


    private void dismissProgessDialog() {
        if (DialogUtil.getHttpialog() != null && DialogUtil.getHttpialog().isShowing()) {
            DialogUtil.getHttpialog().dismiss();
            DialogUtil.setHttpialogNull();
        }
    }

    protected void initProgressDialog(Context context) {
        Dialog dialog = new DialogUtil().getHttpDialog(context, "loading...");
        dialog.setOnCancelListener(this);
        dialog.dismiss();
        if (!dialog.isShowing()) {
            dialog.show();
        }
    }

    @Override
    public void onCancel(DialogInterface dialog) {
        //解除观察者和被观察者的绑定
        finish();
        if (mDisposable != null && !mDisposable.isDisposed()) {
            mDisposable.dispose();
        }
    }

    private void finish() {
        if (context instanceof BaseActivity) {
            BaseActivity b = (BaseActivity) context;
            b.onError();
        }
    }

    public static int getResourceId(Context context, String resourceName, String resourceType) {
        return context.getResources().getIdentifier(resourceName, resourceType,
                context.getPackageName());
    }

    private void showTips(BaseEntity entity) {
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
