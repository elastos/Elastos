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

package org.elastos.wallet.ela.ui.mine.presenter;

import android.content.Context;

import com.blankj.utilcode.util.ZipUtils;

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

import java.io.File;
import java.util.ArrayList;

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
                //moveLogFile(baseFragment.getContext(), "/spvsdk.1.log");
                return new CommmonStringWithiMethNameEntity(MyWallet.SUCCESSCODE, moveLogFile(baseFragment.getContext(), "spvsdk.log") + "", "moveLogFile");
            }
        });
        subscriberObservable(observer, observable, baseFragment);
    }

    private static String moveLogFile(Context context, String logName) {
        String rootPath = context.getFilesDir().getParent();
        File file = new File(rootPath, logName);//原始文件
        if (!file.exists()) {
            return null;
        }
        try {
            File srcFile = context.getExternalFilesDir("log");//目标文件夹
            File srcFile1 = new File(rootPath, "spvsdk.log");//目标文件夹
            File srcFile2 = new File(srcFile, "walletapp.log");//目标文件夹
            //if (srcFile1.exists() || srcFile2.length() > 10485760) {
            //大于10m压缩
            File zipFile = new File(srcFile, "log.zip");
            ArrayList<File> arrayList = new ArrayList<File>();
            arrayList.add(srcFile1);
            arrayList.add(srcFile2);
            ZipUtils.zipFiles(arrayList, zipFile);
            return zipFile.getAbsolutePath();
            //  }
            /*InputStream is = new FileInputStream(file);
            OutputStream fosto = new FileOutputStream(srcFile + File.separator + logName);
            byte bt[] = new byte[1024];
            int c = 0;
            while ((c = is.read(bt)) > 0) {
                fosto.write(bt, 0, c);
            }
            is.close();
            fosto.close();
            return srcFile.getAbsolutePath() + File.separator + logName;*/
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

        File target = context.getExternalFilesDir(root);
        try {
            CopyFile.dirCopy(rootPath + "/" + root, target.getAbsolutePath());
            return target.getAbsolutePath();
        } catch (Exception ex) {
            ex.printStackTrace();
            return null;
        }
    }
}
