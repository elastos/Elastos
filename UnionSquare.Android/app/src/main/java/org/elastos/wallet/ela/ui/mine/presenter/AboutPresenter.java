package org.elastos.wallet.ela.ui.mine.presenter;

import android.content.Context;
import android.os.Environment;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.ElaWallet.WalletNet;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringWithiMethNameEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;
import org.elastos.wallet.ela.utils.CopyFile;
import org.elastos.wallet.ela.utils.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class AboutPresenter extends PresenterAbstract {

    public void getVersion(BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().getVersion();
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    public void moveLogFile(BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                if (MyApplication.currentWalletNet != WalletNet.MAINNET
                        && MyApplication.currentWalletNet != WalletNet.ALPHAMAINNET) {
                    moveWalletFile(baseFragment.getContext());
                }
                moveLogFile(baseFragment.getContext(),"/spvsdk1.log");
                return new CommmonStringWithiMethNameEntity(MyWallet.SUCCESSCODE, moveLogFile(baseFragment.getContext(),"/spvsdk.log") + "", "moveLogFile");
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    private static String moveLogFile(Context context,String logName) {
        String rootPath = context.getFilesDir().getParent();
        File file = new File(rootPath +logName );
        if (!file.exists()) {
            return null;
        }

        try {
            InputStream is = new FileInputStream(file);
            String state = Environment.getExternalStorageState();
            if (!Environment.MEDIA_MOUNTED.equals(state)) {
                return null;
            }
            File file1 = new File(Environment.getExternalStoragePublicDirectory(
                    context.getPackageName()), "log");
            if (file1.mkdirs()) {
                Log.e("moveLogFile", "Directory  created");
            }


            OutputStream fosto = new FileOutputStream(file1 + logName);
            byte bt[] = new byte[1024];
            int c = 0;
            while ((c = is.read(bt)) > 0) {
                fosto.write(bt, 0, c);
            }
            is.close();
            fosto.close();
            return file1.getAbsolutePath() + logName;
        } catch (Exception ex) {
            ex.printStackTrace();
            return null;
        }

    }

    private static String moveWalletFile(Context context) {
        String rootPath = context.getFilesDir().getParent();

        String root = "RegTest";
        switch (MyApplication.currentWalletNet) {
            case WalletNet.TESTNET:
                root = "TestNet";
                break;
            case WalletNet.REGTESTNET:
                root = "RegTest";
                break;
            case WalletNet.PRVNET:
                root = "PrvNet";
                break;

        }

        String target = Environment.getExternalStoragePublicDirectory(
                context.getPackageName()).getAbsolutePath();
        try {
            CopyFile.dirCopy(rootPath + "/" + root, target + "/" + root);
            return target;
        } catch (Exception ex) {
            ex.printStackTrace();
            return null;
        }
    }
}
