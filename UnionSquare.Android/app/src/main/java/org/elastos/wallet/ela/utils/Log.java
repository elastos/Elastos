package org.elastos.wallet.ela.utils;

import org.elastos.wallet.ela.MyApplication;

public class Log {
    private static boolean showLog = MyApplication.chainID != 0;

    public static void v(String tag, String msg) {
        if (showLog)
            android.util.Log.v(tag, msg);
    }

    public static void v(String tag, String msg, Throwable t) {
        if (showLog)
            android.util.Log.v(tag, msg, t);
    }

    public static void d(String tag, String msg) {
        if (showLog)
            android.util.Log.d(tag, msg);
    }

    public static void d(String tag, String msg, Throwable t) {
        if (showLog)
            android.util.Log.d(tag, msg, t);
    }

    public static void i(String tag, Object msg) {
        if (showLog && msg != null)
            android.util.Log.i(tag, msg.toString());
    }

    public static void i(String tag, String msg, Throwable t) {
        if (showLog)
            android.util.Log.i(tag, msg, t);
    }

    public static void w(String tag, String msg) {
        if (showLog)
            android.util.Log.w(tag, msg);
    }

    public static void w(String tag, String msg, Throwable t) {
        if (showLog)
            android.util.Log.w(tag, msg, t);
    }

    public static void e(String tag, String msg) {
        if (showLog)
            android.util.Log.e(tag, msg);
    }

    public static void e(String tag, String msg, Throwable t) {
        if (showLog)
            android.util.Log.e(tag, msg, t);
    }
}

