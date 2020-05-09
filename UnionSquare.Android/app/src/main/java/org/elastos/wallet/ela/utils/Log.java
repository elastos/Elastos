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

package org.elastos.wallet.ela.utils;

import org.elastos.wallet.BuildConfig;
import org.elastos.wallet.ela.ElaWallet.WalletNet;
import org.elastos.wallet.ela.MyApplication;

public class Log {
    private static boolean showLog = MyApplication.currentWalletNet != WalletNet.MAINNET || BuildConfig.DEBUG;

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

