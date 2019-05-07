package org.elastos.wallet.ela.ui.vote.SuperNodeList;

import android.content.Context;
import android.util.Log;

import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;

import io.reactivex.Observable;
import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;

public class SuperNodeListPresenter extends PresenterAbstract {


    private static final String TAG = SuperNodeListPresenter.class.getName();

    //提交投票
    public void getUrlJson(Context context, String url, NodeDotJsonViewData nodeDotJsonViewData) {
        Observer observer = new Observer<NodeInfoBean>() {
            @Override
            public void onSubscribe(Disposable d) {
                Disposable mDisposable = d;
                Log.e(TAG, "onSubscribe");
            }

            @Override
            public void onNext(NodeInfoBean value) {
                Log.e(TAG, "onNext.......:" + value);
                nodeDotJsonViewData.onGetNodeDotJsonData(value);


            }

            @Override
            public void onError(Throwable e) {
                Log.e(TAG, "onError=" + e.getMessage());
            }

            @Override
            public void onComplete() {
                Log.e(TAG, "onComplete()");
            }
        };
        try {
            Observable observable = RetrofitManager.getApiService(context).getUrlJson(url);
            subscriberObservable(observer, observable);
        } catch (Exception e) {
            Log.d("s", e.getMessage());
        }
    }

}
