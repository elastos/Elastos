package org.elastos.wallet.ela.net;

import java.io.IOException;

import okhttp3.Interceptor;
import okhttp3.Request;
import okhttp3.Response;

/**
 * Created by wangdongfeng on 2018/4/20.
 */

public class InterceptorCom implements Interceptor {
    @Override
    public Response intercept(Chain chain) throws IOException {
        Request request = chain.request()
                .newBuilder()
//                .addHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8")
//                .addHeader("Accept-Encoding", "gzip, deflat  e")
//                .addHeader("Connection", "keep-alive")
//                .addHeader("Accept", "*/*")
//                .addHeader("Cookie", "add cookies here")
                .header("Content-Type", "application/json;charset=utf-8")
                .header("Accept", "application/json")
                .build();
        return chain.proceed(request);
    }
}
