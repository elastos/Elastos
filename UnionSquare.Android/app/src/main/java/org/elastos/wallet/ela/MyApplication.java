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

package org.elastos.wallet.ela;

import android.content.Context;
import android.support.multidex.MultiDexApplication;
import android.webkit.WebView;

import org.elastos.wallet.BuildConfig;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.ElaWallet.WalletNet;
import org.elastos.wallet.ela.utils.CrashHandler;
import org.elastos.wallet.ela.utils.SPUtil;

import java.util.HashSet;
import java.util.Set;

import io.realm.Realm;


public class MyApplication extends MultiDexApplication {
    public static MyApplication myApplication;
    public static int currentWalletNet = WalletNet.MAINNET;
    private static MyWallet myWallet;
    public static Set<String> serverList = new HashSet<>();
    public static String REQUEST_BASE_URL;
    // public static List<Activity> activities = new ArrayList<Activity>();


//public  static void

    @Override
    public void onCreate() {
        super.onCreate();
        new WebView(this).destroy();
        myApplication = this;
        serverList.add(WalletNet.MAINURL);
        serverList.add(WalletNet.MAINURL1);
        serverList = new SPUtil(this.getApplicationContext()).getDefaultServerList(serverList);
        REQUEST_BASE_URL = new SPUtil(this.getApplicationContext()).getDefaultServer(serverList.iterator().next());
        // Utils.init(this);
        Realm.init(getApplicationContext());
        String pachageName = getPackageName();
        if (pachageName.endsWith("unionsquare")) {
            currentWalletNet = WalletNet.ALPHAMAINNET;
            useBugly();
        }
        if (pachageName.endsWith("testnet")) {
            currentWalletNet = WalletNet.TESTNET;
            REQUEST_BASE_URL = WalletNet.TESTURL;
            useBugly();
        }
        if (pachageName.endsWith("regtest")) {
            currentWalletNet = WalletNet.REGTESTNET;
            REQUEST_BASE_URL = WalletNet.REGTESTURL;
            useBugly();
        }
        if (pachageName.endsWith("prvNet")) {
            currentWalletNet = WalletNet.PRVNET;
            REQUEST_BASE_URL = WalletNet.PRVURL;
            useBugly();
        }
        CrashHandler.getInstance().init(this);

    }

    private void useBugly() {
        if (!BuildConfig.DEBUG) {
            //CrashReport.initCrashReport(getApplicationContext(), "9c89947c00", false);
        }
    }

    public static String getRoutDir() {
        return getAppContext().getFilesDir().getParent();
    }

    public static Context getAppContext() {
        return myApplication.getApplicationContext();
    }

    public static MyApplication getInstance() {
        return myApplication;
    }

    public static MyWallet getMyWallet() {
        return myWallet;
    }

    public static void setMyWallet(MyWallet myWallet) {
        MyApplication.myWallet = myWallet;
    }

}
