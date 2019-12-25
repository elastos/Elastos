package org.elastos.wallet.ela.ui.main;

import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.net.Uri;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.util.DisplayMetrics;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.FirstFragment;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.ui.main.presenter.MainPresenter;
import org.elastos.wallet.ela.ui.main.viewdata.MainViewData;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.StatusBarUtil;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;


public class MainActivity extends BaseActivity implements MainViewData {


    static {
        System.loadLibrary("spvsdk_jni");
    }

    private boolean flag;


    @Override
    protected int getLayoutId() {
        return R.layout.activity_main;
    }

    @Override
    protected void initView() {
        //initJG();
        Intent mIntent = getIntent();
        String action = mIntent.getAction();
        if (TextUtils.equals(action, Intent.ACTION_VIEW)) {
            Uri uri = mIntent.getData();
            if (TextUtils.equals(uri.getScheme(), "content")) {
                // readFileByBytes(uri);
            }
        }
        init();
        StatusBarUtil.setTranslucentForImageViewInFragment(this, 0, null);
        if (findFragment(FirstFragment.class) == null) {
            loadRootFragment(R.id.mhoneframeLayout, FirstFragment.newInstance());
        }
        flag = false;
        registReceiver();
    }


    public String readFileByBytes(Uri uri) {

        StringBuffer sb = new StringBuffer();

        // 一次读多个字节
        byte[] tempbytes = new byte[1024];
        int byteread = 0;
        try {
            InputStream in = getContentResolver().openInputStream(uri);
            // 读入多个字节到字节数组中，byteread为一次读入的字节数
            while ((byteread = in.read(tempbytes)) != -1) {
                String str = new String(tempbytes, 0, byteread);
                sb.append(str);

            }
            return sb.toString();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
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
        if (MyApplication.getMyWallet() == null) {
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
       // moveTestConfigFiles2RootPath(this);
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
        if (flag) {
            return;
        }
        getWallet().onDestroy();
        getWallet().mMasterWalletManager = null;
        MyApplication.setMyWallet(null);
    }


    @Override
    public void onGetMyWallet(MyWallet myWallet) {
        myWallet.onResume(true);
    }

    public static void moveTestConfigFiles2RootPath(Context context) {
        String rootPath = context.getFilesDir().getParent();
        List<String> names = new ArrayList<String>();
     /*   String name = "Config.cfg";
        switch (MyApplication.chainID) {
            case 1:
                name = "Config_TestNet.cfg";
                break;
            case 2:
                name = "Config_RegTest.cfg";
                break;

        }
        names.add(name);*/
        names.add("mnemonic_chinese.txt");
        names.add("mnemonic_french.txt");
        names.add("mnemonic_italian.txt");
        names.add("mnemonic_japanese.txt");
        names.add("mnemonic_spanish.txt");

       /* List<String> names1 = new ArrayList<String>();
        names1.add("Config.cfg");
        names1.add("mnemonic_chinese.txt");
        names1.add("mnemonic_french.txt");
        names1.add("mnemonic_italian.txt");
        names1.add("mnemonic_japanese.txt");
        names1.add("mnemonic_spanish.txt");*/

        for (int i = 0; i < names.size(); i++) {
            File file = new File(rootPath + "/" + names.get(i));
            if (file.exists()) {
                continue;
            }
            InputStream is = context.getClass().getClassLoader().getResourceAsStream("assets/" + names.get(i));
            try {
                OutputStream fosto = new FileOutputStream(file);
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

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.CHANGELANGUAGE.ordinal()) {

            flag = true;
        }
    }
}
