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


public class PresenterAbstract implements DialogInterface.OnCancelListener {
    private String TAG = getClass().getSimpleName();
    private Disposable mDisposable;
    private boolean isShowDialog = true;
    private Context context;

    protected void subscriberObservable(Observer subscriber,
                                        Observable observable) {
        observable.subscribeOn(Schedulers.io())
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


    protected Observer<BaseEntity> createObserver(Class<? extends SubscriberOnNextLisenner> listener, BaseFragment baseFragment, boolean isShowDialog) {
        //初始化参数
        this.isShowDialog = isShowDialog;
        return createObserver(listener, baseFragment);
    }

    protected Observer<BaseEntity> createObserver(Class<? extends SubscriberOnNextLisenner> listener, BaseActivity baseActivity, boolean isShowDialog) {
        //初始化参数
        this.isShowDialog = isShowDialog;
        return createObserver(listener, baseActivity);
    }

    protected Observer<BaseEntity> createObserver(Class<? extends SubscriberOnNextLisenner> listener, BaseFragment baseFragment) {
        //初始化参数
        this.context = baseFragment.getBaseActivity();
        if (isShowDialog) {
            initProgressDialog(context);
        }
        SubscriberOnNextLisenner lisener = LisenerFactor.create(listener);
        lisener.setViewData((BaseViewData) baseFragment);
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
                if (MyWallet.SUCESSCODE.equals(value.getCode())||"0".equals(value.getCode())) {
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
            }

            @Override
            public void onComplete() {
                Log.e(TAG, "onComplete()");
                finish();
            }
        };


    }

    protected Observer<BaseEntity> createObserver(Class<? extends SubscriberOnNextLisenner> listener, BaseActivity baseActivity) {
        //初始化参数
        this.context = baseActivity;
        if (isShowDialog) {
            initProgressDialog(context);
        }
        SubscriberOnNextLisenner lisener = LisenerFactor.create(listener);
        lisener.setViewData((BaseViewData) baseActivity);
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
                if (MyWallet.SUCESSCODE.equals(value.getCode())) {
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
