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

package org.elastos.wallet.ela.ui.did.fragment;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.content.FileProvider;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.BuildConfig;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.ui.did.presenter.CredencialPresenter;

import java.io.File;
import java.util.Date;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class ExportCredencialFragment extends BaseFragment implements NewBaseViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv)
    TextView tv;
    Unbinder unbinder;
    private Wallet wallet;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_credential_export;
    }

    @Override
    protected void setExtraData(Bundle data) {
        wallet = data.getParcelable("wallet");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.exportcredencial));
        tv.setText(getMyDID().getName(getMyDID().getDIDDocument()) + ".jwt");
    }

    private int type;
    private File curentFile;

    @OnClick({R.id.tv_keep, R.id.tv_share})
    public void onViewClicked(View view) {

        switch (view.getId()) {
            case R.id.tv_keep:
                type = 1;
                curentFile = getCurrentCredentialFile();
                new CredencialPresenter().keepFile(curentFile, getMyDID().getCredentialJSon(getMyDID().getDidString()), this);

                break;
            case R.id.tv_share:
                type = 2;
                curentFile = getCurrentCredentialFile();
                new CredencialPresenter().keepFile(curentFile, getMyDID().getCredentialJSon(getMyDID().getDidString()), this);

                break;


        }
    }


    private File getCurrentCredentialFile() {
        String did = getMyDID().getSpecificDidString();
        File file = getBaseActivity().getExternalFilesDir("credentials" + File.separator + did);
        did = did.substring(did.length() - 6);
        String fileName = did + "_" + new Date().getTime() / 1000 + "_" + getMyDID().getName(getMyDID().getDIDDocument()) + ".jwt";
        //String fileName = getMyDID().getName(getMyDID().getDIDDocument()) + new Date().getTime() / 1000 + ".jwt";
        return new File(file, fileName);

    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "keepFile":
                CommmonBooleanEntity commmonBooleanEntity = (CommmonBooleanEntity) baseEntity;
                if (commmonBooleanEntity.getData()) {
                    showToast(getContext().getString(R.string.savesucess) + curentFile.getAbsolutePath());
                    if (type == 2)
                        shareFile();
                    else {
                        popBackFragment();
                    }
                } else {
                    showToast(getString(R.string.keepfaile));
                }
                break;
        }
    }

    private void shareFile() {
        Intent share_intent = new Intent();
        share_intent.setAction(Intent.ACTION_SEND);//设置分享行为
        share_intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        share_intent.setType("*/*");//设置分享内容的类型
        if (!curentFile.exists()) {
            return;
        }
        Uri uri = getUri(getContext(), BuildConfig.APPLICATION_ID + ".fileProvider", curentFile);

        share_intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION
                | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
        share_intent.putExtra(Intent.EXTRA_STREAM, uri);//添加分享内容
        //  startActivity(share_intent);
        startActivity(Intent.createChooser(share_intent, "share"));
    }

    public static Uri getUri(Context context, String authorites, File file) {
        Uri uri;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            //设置7.0以上共享文件，分享路径定义在xml/file_paths.xml
            uri = FileProvider.getUriForFile(context, authorites, file);
        } else {
            // 7.0以下,共享文件
            uri = Uri.fromFile(file);
        }
        return uri;
    }

}
