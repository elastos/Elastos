package org.elastos.wallet.ela.ui.vote.SuperNodeList;

import android.util.Log;
import android.widget.ImageView;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;

import io.reactivex.Observable;
import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;

public class SuperNodeListPresenter extends PresenterAbstract {


    private static final String TAG = SuperNodeListPresenter.class.getName();

    //提交投票
    public void getUrlJson(String url, BaseFragment baseFragment, NodeDotJsonViewData nodeDotJsonViewData) {
        String tempUrl = url;
        if (!url.startsWith("http")) {
            url = "http://" + url;
        }
        if (url.endsWith("bpinfo.json")) {

        } else if (url.endsWith("/")) {
            url += "bpinfo.json";
        } else {
            url += "/bpinfo.json";
        }
        Observer observer = new Observer<NodeInfoBean>() {
            @Override
            public void onSubscribe(Disposable d) {
                Log.e(TAG, "onSubscribe");
                nodeDotJsonViewData.onSubscribe(tempUrl, d);
            }

            @Override
            public void onNext(NodeInfoBean value) {
                Log.e(TAG, "onNext.......:" + value);
                nodeDotJsonViewData.onGetNodeDotJsonData(value, tempUrl);


            }

            @Override
            public void onError(Throwable e) {
                Log.e(TAG, "onError=" + e.getMessage());
                nodeDotJsonViewData.onError(tempUrl);
            }

            @Override
            public void onComplete() {
                Log.e(TAG, "onComplete()");
            }
        };
        try {
            Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).getUrlJson(url);
            subscriberObservable(observer, observable, baseFragment);
        } catch (Exception e) {
            Log.d("s", e.getMessage());
        }
    }

    public void getUrlJson(ImageView iv, String url, BaseFragment baseFragment, NodeDotJsonViewData nodeDotJsonViewData) {
        String tempUrl = url;
        if (!url.startsWith("http")) {
            url = "http://" + url;
        }
        if (url.endsWith("bpinfo.json")) {

        } else if (url.endsWith("/")) {
            url += "bpinfo.json";
        } else {
            url += "/bpinfo.json";
        }
        Observer observer = new Observer<NodeInfoBean>() {
            @Override
            public void onSubscribe(Disposable d) {
                Log.e(TAG, "onSubscribe");
                nodeDotJsonViewData.onSubscribe(tempUrl, d);
            }

            @Override
            public void onNext(NodeInfoBean value) {
                Log.e(TAG, "onNext.......:" + value);
                nodeDotJsonViewData.onGetNodeDotJsonData(value, tempUrl);
                nodeDotJsonViewData.onGetNodeDotJsonData(iv,value, tempUrl);


            }

            @Override
            public void onError(Throwable e) {
                Log.e(TAG, "onError=" + e.getMessage());
                nodeDotJsonViewData.onError(tempUrl);
            }

            @Override
            public void onComplete() {
                Log.e(TAG, "onComplete()");
            }
        };
        try {
            Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).getUrlJson(url);
            subscriberObservable(observer, observable, baseFragment);
        } catch (Exception e) {
            Log.d("s", e.getMessage());
        }
    }


}
