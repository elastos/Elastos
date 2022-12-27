/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.wallet.ela.rxjavahelp;

import android.app.Dialog;
import android.widget.Toast;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.Log;

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
                if (MyWallet.SUCCESSCODE.equals(value.getCode()) || "0".equals(value.getCode()) || "200".equals(value.getCode()) || "1".equals(value.getCode())
                        || MyWallet.errorCodeDoInMeathed.equals(value.getCode())) {
                    ((NewBaseViewData) baseFragment).onGetData(methodName, value, o);
                } else {
                    showTips(value);
                }
                Log.e(TAG, methodName + " onNext:" + value);

            }

            @Override
            public void onError(Throwable e) {
                if (isShowDialog) {
                    dismissProgessDialog(dialog);
                }
                Log.e(TAG, "onError=" + e.getMessage());
                try {
                    Toast.makeText(context, e.getMessage(), Toast.LENGTH_SHORT).show();
                } catch (Exception e1) {
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
                if (MyWallet.SUCCESSCODE.equals(value.getCode()) || "0".equals(value.getCode())
                        || MyWallet.errorCodeDoInMeathed.equals(value.getCode())) {
                    ((NewBaseViewData) baseActivity).onGetData(methodName, value, o);
                } else {
                    showTips(value);

                }
                Log.e(TAG, methodName + " onNext:" + value);

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
