package org.elastos.wallet.ela;

import android.content.Context;
import android.support.multidex.MultiDexApplication;
import android.webkit.WebView;

import com.tencent.bugly.crashreport.CrashReport;

import org.elastos.wallet.BuildConfig;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.ElaWallet.WalletNet;
import org.elastos.wallet.ela.utils.SPUtil;

import java.util.HashSet;
import java.util.Set;

import io.realm.Realm;


public class MyApplication extends MultiDexApplication {
    private static MyApplication myApplication;
    public static int currentWalletNet = WalletNet.MAINNET;
    protected static MyWallet myWallet;
    public static Set<String> serverList = new HashSet<>();
    public static String REQUEST_BASE_URL;

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
    }

    private void useBugly() {
        if (!BuildConfig.DEBUG) {
            CrashReport.initCrashReport(getApplicationContext(), "9c89947c00", false);
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