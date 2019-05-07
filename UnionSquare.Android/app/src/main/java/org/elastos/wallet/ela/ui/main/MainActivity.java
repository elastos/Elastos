package org.elastos.wallet.ela.ui.main;

import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.util.DisplayMetrics;

import org.elastos.wallet.ela.ElaWallet.MyUtil;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.FirstFragment;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.ui.main.presenter.MainPresenter;
import org.elastos.wallet.ela.ui.main.viewdata.MainViewData;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.StatusBarUtil;


import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Locale;

import static org.elastos.wallet.ela.MyApplication.chainID;


public class MainActivity extends BaseActivity implements MainViewData {


    static {
        System.loadLibrary("spvsdk_jni");
    }


    @Override
    protected int getLayoutId() {
        return R.layout.activity_main;
    }

    @Override
    protected void initView() {
        //initJG();
        init();
        StatusBarUtil.setTranslucentForImageViewInFragment(this, 0, null);
        if (findFragment(FirstFragment.class) == null) {
            loadRootFragment(R.id.mhoneframeLayout, FirstFragment.newInstance());
        }
    }

    @Override
    protected void initInjector() {

    }

    @Override
    public void onBackPressedSupport() {
        // 对于 4个类别的主Fragment内的回退back逻辑,已经在其onBackPressedSupport里各自处理了
        super.onBackPressedSupport();
    }

//    @Override
//    public FragmentAnimator onCreateFragmentAnimator() {
//        // 设置横向(和安卓4.x动画相同)
//        return new DefaultHorizontalAnimator();
//    }


    @Override
    protected void onPause() {
        super.onPause();
        getWallet().onPause(true);

    }

    @Override
    protected void onResume() {

        super.onResume();
        if (myWallet == null) {

            new MainPresenter().getWallet(this);
        } else {
            getWallet().onResume(true);
        }
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // changeAppLanguage();
        setLanguage();
    }

    private void init() {
        if (chainID == 0) {
            MyUtil.moveConfigFiles2RootPath(this);
        } else if (chainID == 1) {
            moveTestConfigFiles2RootPath(this);
        }
        Context applicationContext = getApplicationContext();
        MyUtil.setApplicationContext(applicationContext);
    }

    private void setLanguage() {
        if (new SPUtil(this).getLanguage() == -1) {
            if (Locale.getDefault().getLanguage().equals("zh")) {
                new SPUtil(this).setLanguage(0);
            } else if (Locale.getDefault().getLanguage().equals("en")) {
                new SPUtil(this).setLanguage(1);
            } else {
                new SPUtil(this).setLanguage(1);
                changeAppLanguage();
            }
        } else {
            changeAppLanguage();
        }

    }

    public void changeAppLanguage() {

        String sta = new SPUtil(this).getLanguage() == 0 ? "zh" : "en";//这是SharedPreferences工具类，用于保存设置，代码很简单，自己实现吧
        // 本地语言设置
        Locale myLocale = new Locale(sta);
        Resources res = getResources();
        DisplayMetrics dm = res.getDisplayMetrics();
        Configuration conf = res.getConfiguration();
        conf.locale = myLocale;
        res.updateConfiguration(conf, dm);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        getWallet().onDestroy();
        getWallet().mMasterWalletManager = null;
        myWallet = null;
    }


    @Override
    public void onGetMyWallet(MyWallet myWallet) {
        myWallet.onResume(true);
    }

    public static void moveTestConfigFiles2RootPath(Context context) {
        String rootPath = context.getFilesDir().getParent();
        String[] names = {"CoinConfig_TestNet.json",
                "mnemonic_chinese.txt",
                "mnemonic_french.txt",
                "mnemonic_italian.txt",
                "mnemonic_japanese.txt",
                "mnemonic_spanish.txt"};
        String[] names1 = {"CoinConfig.json",
                "mnemonic_chinese.txt",
                "mnemonic_french.txt",
                "mnemonic_italian.txt",
                "mnemonic_japanese.txt",
                "mnemonic_spanish.txt"};
        for (int i = 0; i < names.length; i++) {
            InputStream is = context.getClass().getClassLoader().getResourceAsStream("assets/" + names[i]);
            try {
                OutputStream fosto = new FileOutputStream(rootPath + "/" + names1[i]);
                byte bt[] = new byte[1024];
                int c = 0;
                while ((c = is.read(bt)) > 0) {
                    fosto.write(bt, 0, c);
                }
                is.close();
                fosto.close();

            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    }
}
