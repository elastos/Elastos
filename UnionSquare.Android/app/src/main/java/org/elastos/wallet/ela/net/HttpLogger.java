package org.elastos.wallet.ela.net;


import org.elastos.wallet.ela.utils.klog.KLog;

import okhttp3.logging.HttpLoggingInterceptor;

public class HttpLogger implements HttpLoggingInterceptor.Logger {
    @Override
    public void log(String message) {
        KLog.a(message);
    }
}


