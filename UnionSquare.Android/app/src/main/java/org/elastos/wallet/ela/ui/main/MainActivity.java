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

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.FirstFragment;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.presenter.CredencialPresenter;
import org.elastos.wallet.ela.ui.main.presenter.MainPresenter;
import org.elastos.wallet.ela.ui.main.viewdata.MainViewData;
import org.elastos.wallet.ela.ui.mine.fragment.MessageListFragment;
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
import java.util.Date;
import java.util.List;
import java.util.Locale;


public class MainActivity extends BaseActivity implements MainViewData, NewBaseViewData {

    @Override
    protected void onNewIntent(Intent intent) {

        getNotify(intent);
        setIntent(intent);
        super.onNewIntent(intent);
    }

    /**
     * 点击通知栏到主Activity时 会执行这个方法
     *
     * @param intent
     */
    private void getNotify(Intent intent) {
        String value = intent.getStringExtra("toValue");
        if (!TextUtils.isEmpty(value)) {
            switch (value) {
                //在进行判断需要做的操作
                case "notice":
                    if (getTopFragment().getClass() == MessageListFragment.class) {
                        post(RxEnum.REFRESHMESSAGE.ordinal(), null, null);
                        return;
                    }
                    ((BaseFragment) getTopFragment()).start(MessageListFragment.class);
                    break;
            }
        }

    }

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
        String toValue = mIntent.getStringExtra("toValue");
        StatusBarUtil.setTranslucentForImageViewInFragment(this, 0, null);
        if (findFragment(FirstFragment.class) == null) {
            FirstFragment firstFragment = new FirstFragment();
            if (!TextUtils.isEmpty(toValue)) {
                Bundle bundle = new Bundle();
                bundle.putString("toValue", toValue);
                firstFragment.setArguments(bundle);
            }
            loadRootFragment(R.id.mhoneframeLayout, firstFragment);
        }
        flag = false;
        registReceiver();
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
        Intent mIntent = getIntent();
        String action = mIntent.getAction();
        if (TextUtils.equals(action, Intent.ACTION_VIEW)) {
            Uri uri = mIntent.getData();
            if (uri == null) {
                return;
            }
            if (!uri.toString().endsWith(".jwt")) {
                showToastMessage(getString(R.string.keepfaile));
            } else {
                mIntent.setData(null);
                new MainPresenter().readUri(uri, this);
            }


        }
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // changeAppLanguage();
        setLanguage();
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
        if (getMyWallet() != null) {
            getWallet().onDestroy();
            getWallet().mMasterWalletManager = null;
            MyApplication.setMyWallet(null);
        }
    }

    /*@Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        if (getMyWallet() != null) {
            getWallet().onDestroy();
            getWallet().mMasterWalletManager = null;
            MyApplication.setMyWallet(null);
        }
        super.onRestoreInstanceState(savedInstanceState);
    }*/

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

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "keepFile":
                CommmonBooleanEntity commmonBooleanEntity = (CommmonBooleanEntity) baseEntity;
                if (commmonBooleanEntity.getData()) {
                    showToastMessage(getString(R.string.savesucess));
                } else {
                    showToastMessage(getString(R.string.keepfaile));
                }
                break;
            case "readUri":
                CommmonStringEntity entity = (CommmonStringEntity) baseEntity;
                String credentialJson = entity.getData();
                String pro = getMyDID().getCredentialProFromJson(credentialJson);
                CredentialSubjectBean credentialSubjectBean = JSON.parseObject(pro, CredentialSubjectBean.class);
                if (credentialSubjectBean != null) {
                    String didName = credentialSubjectBean.getDidName();
                    if (TextUtils.isEmpty(didName)) {
                        didName = "unknown";
                    }
                    String did = credentialSubjectBean.getDid().replace("did:elastos:", "");
                    new CredencialPresenter().keepFile(getCurrentCredentialFile(did, didName), credentialJson, this);
                }
                break;
        }


    }

    private File getCurrentCredentialFile(String did, String didName) {
        File file = getExternalFilesDir("credentials" + File.separator + did);
        did = did.substring(did.length() - 6);
        String fileName = did + "_" + new Date().getTime() / 1000 + "_" + didName + ".jwt";
        return new File(file, fileName);

    }
}
