package org.elastos.wallet.ela.ui.vote.SuperNodeList;

import android.text.TextUtils;
import android.widget.ImageView;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.ImageBean;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;
import org.elastos.wallet.ela.ui.did.entity.GetJwtRespondBean;
import org.elastos.wallet.ela.utils.Log;

import java.util.HashMap;
import java.util.Map;

import io.reactivex.Observable;
import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;

public class SuperNodeListPresenter extends NewPresenterAbstract {
    public void getImage(ImageView iv, String url, String imageurl, BaseFragment baseFragment, NodeDotJsonViewData nodeDotJsonViewData) {
        Observer observer = new Observer<ImageBean>() {
            @Override
            public void onSubscribe(Disposable d) {
                Log.e(TAG, "onSubscribe");

            }

            @Override
            public void onNext(ImageBean value) {
                Log.e(TAG, "onNext.......:" + value);
                if (!TextUtils.isEmpty(value.getData())) {
                    nodeDotJsonViewData.onGetImage(iv, url, value);
                }
            }

            @Override
            public void onError(Throwable throwable) {
                Log.e(TAG, "onError=" + throwable.getMessage());
                nodeDotJsonViewData.onError(url);
            }


            @Override
            public void onComplete() {
                Log.e(TAG, "onComplete()");
            }
        };
        try {
            Map<String, String> map = new HashMap();
            map.put("imageurl", imageurl);
            Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).getImageUrl(map);
            subscriberObservable(observer, observable, baseFragment);
        } catch (Exception e) {
            Log.d("s", e.getMessage());
        }
    }

    private static final String TAG = SuperNodeListPresenter.class.getName();

    //deposit提交投票
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

    //deposit提交投票
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
                nodeDotJsonViewData.onGetNodeDotJsonData(iv, value, tempUrl);


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

    public void getCRUrlJson(ImageView iv, String did, BaseFragment baseFragment, NodeDotJsonViewData nodeDotJsonViewData) {
        if (TextUtils.isEmpty(did)) {
            return;

        }
        Observer observer = new Observer<GetJwtRespondBean>() {
            @Override
            public void onSubscribe(Disposable d) {
                Log.e(TAG, "onSubscribe");
            }

            @Override
            public void onNext(GetJwtRespondBean value) {
                Log.e(TAG, "onNext.......:" + value);
                nodeDotJsonViewData.onGetNodeDotJsonData(iv, value, did);
            }

            @Override
            public void onError(Throwable e) {
                Log.e(TAG, "onError=" + e.getMessage());
                nodeDotJsonViewData.onError(did);
            }

            @Override
            public void onComplete() {
                Log.e(TAG, "onComplete()");
            }
        };
        try {
            Map<String, String> map = new HashMap();
            map.put("did", did);
            Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).jwtGet(map);
            subscriberObservable(observer, observable, baseFragment);
        } catch (Exception e) {
            Log.d("s", e.getMessage());
        }
    }


}
