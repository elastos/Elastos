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

package org.elastos.wallet.ela.net;

import android.content.Context;

import com.blankj.utilcode.util.NetworkUtils;

import org.elastos.wallet.BuildConfig;
import org.elastos.wallet.ela.ElaWallet.WalletNet;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.utils.SPUtil;

import java.io.File;
import java.io.IOException;
import java.util.concurrent.TimeUnit;

import okhttp3.Cache;
import okhttp3.CacheControl;
import okhttp3.Interceptor;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;
import okhttp3.logging.HttpLoggingInterceptor;
import retrofit2.Retrofit;
import retrofit2.adapter.rxjava2.RxJava2CallAdapterFactory;
import retrofit2.converter.gson.GsonConverterFactory;


public class RetrofitManager {
    private static long CONNECT_TIMEOUT = 30L;
    private static long READ_TIMEOUT = 30L;
    private static long WRITE_TIMEOUT = 30L;
    //设缓存有效期为1天
    private static final long CACHE_STALE_SEC = 60 * 60 * 24 * 1;
    //查询缓存的Cache-Control设置，为if-only-cache时只查询缓存而不会请求服务器，max-stale可以配合设置缓存失效时间
    public static final String CACHE_CONTROL_CACHE = "only-if-cached, max-stale=" + CACHE_STALE_SEC;
    //查询网络的Cache-Control设置
    //(假如请求了服务器并在a时刻返回响应结果，则在max-age规定的秒数内，浏览器将不会发送对应的请求到服务器，数据由缓存直接返回)
    public static final String CACHE_CONTROL_NETWORK = "Cache-Control: public, max-age=10";
    // 避免出现 HTTP 403 Forbidden，参考：http://stackoverflow.com/questions/13670692/403-forbidden-with-java-but-not-web-browser
    private static final String AVOID_HTTP403_FORBIDDEN = "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.11 (KHTML, like Gecko) Chrome/23.0.1271.95 Safari/537.11";
    private static volatile OkHttpClient mOkHttpClient;

    public static ApiServer getApiService1() {
        return apiService1;
    }

    private static ApiServer apiService1;

    public static synchronized ApiServer getApiService(Context context) {
        if (MyApplication.currentWalletNet == WalletNet.MAINNET
                || MyApplication.currentWalletNet == WalletNet.ALPHAMAINNET) {
            String address = new SPUtil(context).getDefaultServer(MyApplication.serverList.iterator().next());
            if (!address.equals(MyApplication.REQUEST_BASE_URL)) {
                //主网高可用导致差异 强制刷新apiService1
                MyApplication.REQUEST_BASE_URL = address;
                createApiService(context);
                return apiService1;
            }
        }
        if (apiService1 == null) {
            createApiService(context);
        }
        return apiService1;
    }


    private static void createApiService(Context context) {
        apiService1 = create(ApiServer.class, context);
    }


    /**
     * 云端响应头拦截器，用来配置缓存策略
     * Dangerous interceptor that rewrites the server's cache-control header.
     */
    private static final Interceptor mRewriteCacheControlInterceptor = new Interceptor() {
        @Override
        public Response intercept(Chain chain) throws IOException {
            Request request = chain.request();
            if (!NetworkUtils.isConnected()) {
                request = request.newBuilder()
                        .cacheControl(CacheControl.FORCE_CACHE)
                        .build();
            }
            Response originalResponse = chain.proceed(request);
            if (NetworkUtils.isConnected()) {
                //有网的时候读接口上的@Headers里的配置，可以在这里进行统一的设置
                String cacheControl = request.cacheControl().toString();
                return originalResponse.newBuilder()
                        .header("Cache-Control", cacheControl)
                        .removeHeader("Pragma")
                        .build();
            } else {
                return originalResponse.newBuilder()
                        .header("Cache-Control", "public, only-if-cached, max-stale=" + CACHE_CONTROL_CACHE)
                        .removeHeader("Pragma")
                        .build();
            }
        }
    };


    /**
     * 获取OkHttpClient实例
     *
     * @return
     */
    public static OkHttpClient getOkHttpClient(Context context) {
        HttpLoggingInterceptor logInterceptor = new HttpLoggingInterceptor(new HttpLogger());
        if (BuildConfig.DEBUG) {
            logInterceptor.setLevel(HttpLoggingInterceptor.Level.BODY);
        } else {
            logInterceptor.setLevel(HttpLoggingInterceptor.Level.NONE);
        }
        OkHttpClient.Builder builder = new OkHttpClient.Builder();
        if (MyApplication.currentWalletNet == WalletNet.TESTNET) {
            try {
                OkhttpManager.getInstance().setTrustrCertificates(context.getAssets().open("server.cer"));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        builder = OkhttpManager.getInstance().build();
        if (mOkHttpClient == null) {
            synchronized (RetrofitManager.class) {
                Cache cache = new Cache(new File(MyApplication.getAppContext().getCacheDir(), "HttpCache"), 1024 * 1024 * 100);
                if (mOkHttpClient == null) {
                    mOkHttpClient = builder//.cache(cache)
                            .connectTimeout(CONNECT_TIMEOUT, TimeUnit.SECONDS)
                            .readTimeout(READ_TIMEOUT, TimeUnit.SECONDS)
                            .writeTimeout(WRITE_TIMEOUT, TimeUnit.SECONDS)
                            // .addInterceptor(mRewriteCacheControlInterceptor)
                            .addInterceptor(logInterceptor)
                            .addInterceptor(new InterceptorCom())
                            /*  .cookieJar(new CookiesManager())*/
                            .build();
                }
            }
        }
        return mOkHttpClient;
    }


    /**
     * 获取OkHttpClient实例
     *
     * @return
     */
    private static OkHttpClient getOkHttpClient() {
        HttpLoggingInterceptor logInterceptor = new HttpLoggingInterceptor(new HttpLogger());
        if (BuildConfig.DEBUG) {
            logInterceptor.setLevel(HttpLoggingInterceptor.Level.BODY);
        } else {
            logInterceptor.setLevel(HttpLoggingInterceptor.Level.NONE);
        }
        OkHttpClient.Builder builder = new OkHttpClient.Builder();
        if (mOkHttpClient == null) {
            synchronized (RetrofitManager.class) {
                Cache cache = new Cache(new File(MyApplication.getAppContext().getCacheDir(), "HttpCache"), 1024 * 1024 * 100);
                if (mOkHttpClient == null) {
                    mOkHttpClient = builder//.cache(cache)
                            .connectTimeout(CONNECT_TIMEOUT, TimeUnit.SECONDS)
                            .readTimeout(READ_TIMEOUT, TimeUnit.SECONDS)
                            .writeTimeout(WRITE_TIMEOUT, TimeUnit.SECONDS)
                            //.addInterceptor(mRewriteCacheControlInterceptor)
                            .addInterceptor(logInterceptor)
                            .addInterceptor(new InterceptorCom())
                            /*  .cookieJar(new CookiesManager())*/
                            .build();
                }
            }
        }
        return mOkHttpClient;
    }

    /**
     * 获取Service
     *
     * @param clazz
     * @param <T>
     * @return
     */
    private static <T> T create(Class<T> clazz, Context context) {
        Retrofit retrofit;
        Retrofit.Builder build = new Retrofit.Builder().baseUrl(MyApplication.REQUEST_BASE_URL)
                .addConverterFactory(GsonConverterFactory.create())
                .addCallAdapterFactory(RxJava2CallAdapterFactory.create());

        retrofit = build.client(getOkHttpClient(context)).build();

        return retrofit.create(clazz);
    }

    private static <T> T create(Class<T> clazz) {
        Retrofit retrofit = new Retrofit.Builder().baseUrl(MyApplication.REQUEST_BASE_URL)
                .client(getOkHttpClient())
                .addConverterFactory(GsonConverterFactory.create())
                .addCallAdapterFactory(RxJava2CallAdapterFactory.create()).build();
        return retrofit.create(clazz);
    }


    public static ApiServer specialCreate() {
        Retrofit retrofit;
        Retrofit.Builder build = new Retrofit.Builder().baseUrl(WalletNet.SERVERLIST_BASE)
                .addConverterFactory(GsonConverterFactory.create())
                .addCallAdapterFactory(RxJava2CallAdapterFactory.create());
        HttpLoggingInterceptor logInterceptor = new HttpLoggingInterceptor(new HttpLogger());
        logInterceptor.setLevel(HttpLoggingInterceptor.Level.BODY);
        OkHttpClient.Builder builder = new OkHttpClient.Builder();

        synchronized (RetrofitManager.class) {
            OkHttpClient mOkHttpClient = builder//.cache(cache)
                    .connectTimeout(2, TimeUnit.SECONDS)
                    .readTimeout(2, TimeUnit.SECONDS)
                    .writeTimeout(2, TimeUnit.SECONDS)
                    .addInterceptor(logInterceptor)
                    .addInterceptor(new InterceptorCom())
                    .build();
            retrofit = build.client(mOkHttpClient).build();
            return retrofit.create(ApiServer.class);
        }

    }

    private static ApiServer webApiService;

    public static ApiServer webApiCreate() {
        if (webApiService != null) {
            return webApiService;
        }
        String baseUrl = WalletNet.WEBURlTEST;
        if (MyApplication.currentWalletNet == WalletNet.ALPHAMAINNET || MyApplication.currentWalletNet == WalletNet.MAINNET) {
            baseUrl = WalletNet.WEBURlPUBLIC;
        }
        Retrofit retrofit;
        Retrofit.Builder build = new Retrofit.Builder().baseUrl(baseUrl)
                .addConverterFactory(GsonConverterFactory.create())
                .addCallAdapterFactory(RxJava2CallAdapterFactory.create());
        HttpLoggingInterceptor logInterceptor = new HttpLoggingInterceptor(new HttpLogger());
        logInterceptor.setLevel(HttpLoggingInterceptor.Level.BODY);
        OkHttpClient.Builder builder = new OkHttpClient.Builder();

        synchronized (RetrofitManager.class) {
            OkHttpClient mOkHttpClient = builder//.cache(cache)
                    .connectTimeout(CONNECT_TIMEOUT, TimeUnit.SECONDS)
                    .readTimeout(READ_TIMEOUT, TimeUnit.SECONDS)
                    .writeTimeout(WRITE_TIMEOUT, TimeUnit.SECONDS)
                    .addInterceptor(logInterceptor)
                    .addInterceptor(new InterceptorCom())
                    .build();
            retrofit = build.client(mOkHttpClient).build();
            webApiService = retrofit.create(ApiServer.class);
            return webApiService;
        }

    }
}
