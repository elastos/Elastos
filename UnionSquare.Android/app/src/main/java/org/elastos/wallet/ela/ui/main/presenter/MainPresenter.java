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

package org.elastos.wallet.ela.ui.main.presenter;

import android.net.Uri;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.ui.common.bean.CommmonObjEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonObjectWithMethNameEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonObjectWithMethNameListener;
import org.elastos.wallet.ela.ui.main.entity.ServerListEntity;
import org.elastos.wallet.ela.ui.main.listener.MyWalletListener;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.PingUtil;
import org.elastos.wallet.ela.utils.SPUtil;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import io.reactivex.Observable;
import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;

public class MainPresenter extends NewPresenterAbstract {

    private static final String TAG = MainPresenter.class.getSimpleName();

    public void getWallet(BaseActivity baseActivity) {
        Observer observer = createObserver(MyWalletListener.class, baseActivity, false);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                MyWallet myWallet = baseActivity.getWallet();
                return new CommmonObjEntity(MyWallet.SUCCESSCODE, myWallet);
            }
        });
        subscriberObservable(observer, observable);
    }


    /*public void getServerList(BaseFragment baseFragment) {
        Observable<ServerListEntity> observable = RetrofitManager.specialCreate().getServerList();
        Observer observer = createObserver(ServerCommonObjectWithMNListener.class, baseFragment, false, "getServerList");
        subscriberObservable(observer, observable, baseFragment);
    }*/

    public void ping(BaseFragment baseFragment, List<String> list, String defaultAdd) {
        Observer observer = createObserver(CommonObjectWithMethNameListener.class, baseFragment, false, "ping");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                String result = PingUtil.ping(list, defaultAdd);
                return new CommmonObjectWithMethNameEntity(MyWallet.SUCCESSCODE, result, "ping");

            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void getServerList(BaseFragment baseFragment) {

        Observer observer = new Observer<BaseEntity>() {
            @Override
            public void onSubscribe(Disposable d) {
            }

            @Override
            public void onNext(BaseEntity value) {
                ServerListEntity serverListEntity = (ServerListEntity) value;
                if ("0".equals(serverListEntity.getCode())) {
                    List<String> list = serverListEntity.getData();
                    MyApplication.serverList = new HashSet<>(list);
                    new SPUtil(baseFragment.getContext()).setDefaultServerList(MyApplication.serverList);
                    ping(baseFragment, list, MyApplication.REQUEST_BASE_URL);

                } else {
                    ping(baseFragment, new ArrayList<>(MyApplication.serverList), MyApplication.REQUEST_BASE_URL);
                }

            }

            @Override
            public void onError(Throwable e) {
                ping(baseFragment, new ArrayList<>(MyApplication.serverList), MyApplication.REQUEST_BASE_URL);
            }

            @Override
            public void onComplete() {
                Log.e(TAG, "onComplete()");
            }
        };
        Observable observable = RetrofitManager.specialCreate().getServerList();
        subscriberObservable(observer, observable, baseFragment);
    }

    public void readUri(Uri uri, BaseActivity baseActivity) {
        Observer observer = createObserver(baseActivity, "readUri");
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                String result = readUriFile(uri, baseActivity);
                return new CommmonStringEntity(MyWallet.SUCCESSCODE, result);

            }
        });
        subscriberObservable(observer, observable, baseActivity);
    }

    public String readUriFile(Uri uri, BaseActivity baseActivity) {

        StringBuffer sb = new StringBuffer();

        // 一次读多个字节
        byte[] tempbytes = new byte[1024];
        int byteread = 0;
        try {
            InputStream in = baseActivity.getContentResolver().openInputStream(uri);
            // 读入多个字节到字节数组中，byteread为一次读入的字节数
            while ((byteread = in.read(tempbytes)) != -1) {
                String str = new String(tempbytes, 0, byteread);
                sb.append(str);

            }
            return sb.toString();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }
}
