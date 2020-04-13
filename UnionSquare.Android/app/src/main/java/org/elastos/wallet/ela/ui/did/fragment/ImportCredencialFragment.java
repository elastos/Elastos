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

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.did.adapter.ImportCredencialRecAdapetr;
import org.elastos.wallet.ela.ui.did.presenter.CredencialPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VertifyPwdActivity;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.FileChooseUtil;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.io.File;
import java.util.Date;

import butterknife.BindView;
import butterknife.OnClick;

public class ImportCredencialFragment extends BaseFragment implements NewBaseViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_title_right)
    TextView tvTitleRight;
    @BindView(R.id.rv)
    RecyclerView rv;
    private Wallet wallet;
    private File files;
    private String credentialJson;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_credential_import;
    }

    @Override
    protected void setExtraData(Bundle data) {
        wallet = data.getParcelable("wallet");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.importcredencial));
        tvTitleRight.setVisibility(View.VISIBLE);
        tvTitleRight.setText(R.string.sure);
        getFileList();
        registReceiver();

    }

    private void getFileList() {
        files = getBaseActivity().getExternalFilesDir("credentials" + File.separator + getMyDID().getSpecificDidString());
        if (!files.exists() || !files.isDirectory()) {
            return;
        }
        setRecycleView();
    }

    private void setRecycleView() {
        ImportCredencialRecAdapetr adapterShow = new ImportCredencialRecAdapetr(getContext(), files.listFiles());
        rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rv.setAdapter(adapterShow);
    }

    @OnClick({R.id.tv_chose, R.id.tv_title_right})
    public void onViewClicked(View view) {

        switch (view.getId()) {
            case R.id.tv_chose:

                requstManifestPermission(getString(R.string.needpermissionstorage));
                break;
            case R.id.tv_title_right:
                int chosePosition = ((ImportCredencialRecAdapetr) rv.getAdapter()).getChosePosition();
                if (chosePosition == -1) {
                    showToast(getString(R.string.please_select));
                } else {
                    File file = files.listFiles()[chosePosition];
                    Log.i("??", file.getName());
                    new CredencialPresenter().readFile(file, this);

                }
                break;


        }
    }


    @Override
    protected void requstPermissionOk() {
        openSystemFile();
    }

    boolean keep = false;

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 1 && resultCode == Activity.RESULT_OK) {
            Uri uri = data.getData();
            try {
                String chooseFilePath = FileChooseUtil.getInstance(getContext()).getChooseFileResultPath(uri);
                Log.d("??", "选择文件返回：" + chooseFilePath);
                File file = new File(chooseFilePath);
                if (!file.getName().endsWith(".jwt")) {
                    showToast(getString(R.string.importfailed));
                } else {
                    //读取并保存本地
                    new CredencialPresenter().readFile(file, this);
                    keep = true;
                }
            } catch (Exception e) {
                showToast(getString(R.string.importfailed));
            }
        }
    }


    public void openSystemFile() {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("*/*");
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        try {
            startActivityForResult(Intent.createChooser(intent, "chose"), 1);
        } catch (ActivityNotFoundException e) {
            e.printStackTrace();
            Toast.makeText(getContext(), getString(R.string.pleaseinstallfilechose), Toast.LENGTH_SHORT);
        }
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "readFile":
                new DialogUtil().showWarmPrompt1(getBaseActivity(), getString(R.string.covercredencial), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        CommmonStringEntity entity = (CommmonStringEntity) baseEntity;
                        credentialJson = entity.getData();
                        //输入密码
                        Intent intent = new Intent(getActivity(), VertifyPwdActivity.class);
                        intent.putExtra("walletId", wallet.getWalletId());
                        intent.putExtra("type", this.getClass().getSimpleName());
                        startActivity(intent);
                    }
                });

                break;
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();

        if (integer == RxEnum.VERTIFYPAYPASS.ordinal()) {
            //验证密码成功
            boolean sucess = getMyDID().restoreCredential(credentialJson, (String) result.getObj(), getMyDID().getExpires(getMyDID().getDIDDocument()));
            if (sucess) {
                //保存到sdk成功了
                if (keep) {
                    //保存到本地
                    new CredencialPresenter().keepFile(getCurrentCredentialFile(), credentialJson, this);
                }
                post(RxEnum.EDITPERSONALINFO.ordinal(), null, null);
                popBackFragment();
            } else {
                showToast(getString(R.string.importfailed));
            }
            keep = false;
        }

    }

    private File getCurrentCredentialFile() {
        String did = getMyDID().getSpecificDidString();
        File file = getBaseActivity().getExternalFilesDir("credentials" + File.separator + did);
        did = did.substring(did.length() - 6);
        //    String fileName =  + new Date().getTime() / 1000 + ".jwt";
        String fileName = did + "_" + new Date().getTime() / 1000 + "_" + getMyDID().getName(getMyDID().getDIDDocument()) + ".jwt";

        return new File(file, fileName);

    }
}
