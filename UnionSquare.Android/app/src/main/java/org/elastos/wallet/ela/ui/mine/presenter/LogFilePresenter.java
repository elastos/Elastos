package org.elastos.wallet.ela.ui.mine.presenter;

import android.content.Context;
import android.os.Environment;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.Assets.listener.GetSupportedChainsListner;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringWithiMethNameEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonStringWithiMethNameListener;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class LogFilePresenter extends PresenterAbstract {


    public void moveLogFile(BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringWithiMethNameListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return new CommmonStringWithiMethNameEntity(MyWallet.SUCCESSCODE, moveLogFile(baseFragment.getContext()) + "", "moveLogFile");
            }
        });
        subscriberObservable(observer, observable);
    }

    private static String moveLogFile(Context context) {
        String rootPath = context.getFilesDir().getParent();
        File file = new File(rootPath + "/spvsdk.log");
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


            OutputStream fosto = new FileOutputStream(file1 + "/spvsdk.log");
            byte bt[] = new byte[1024];
            int c = 0;
            while ((c = is.read(bt)) > 0) {
                fosto.write(bt, 0, c);
            }
            is.close();
            fosto.close();
            return file1.getAbsolutePath()+"/spvsdk.log";
        } catch (Exception ex) {
            ex.printStackTrace();
            return null;
        }

    }
}
