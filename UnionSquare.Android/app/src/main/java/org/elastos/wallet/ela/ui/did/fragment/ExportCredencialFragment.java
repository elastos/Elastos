package org.elastos.wallet.ela.ui.did.fragment;

import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.StrictMode;
import android.provider.MediaStore;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.ui.did.presenter.CredencialPresenter;

import java.io.File;
import java.util.Calendar;

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
                new CredencialPresenter().keepFile(curentFile, getMyDID().getCredentialProFromStore(getMyDID().getDidString()), this);

                break;
            case R.id.tv_share:
                type = 2;
                requstManifestPermission(getString(R.string.needpermissionstorage));
                break;


        }
    }

    @Override
    protected void requstPermissionOk() {
        curentFile = getCurrentCredentialFile();
        new CredencialPresenter().keepFile(curentFile, getMyDID().getCredentialProFromStore(getMyDID().getDidString()), this);

    }


    private File getCurrentCredentialFile() {
        File file = getBaseActivity().getExternalFilesDir("credentials" + File.separator + wallet.getWalletId());
        String fileName = getMyDID().getName(getMyDID().getDIDDocument()) + Calendar.getInstance().get(Calendar.SECOND) + ".jwt";
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
        checkFileUriExposure();
        Intent share_intent = new Intent();
        share_intent.setAction(Intent.ACTION_SEND);//设置分享行为
        share_intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        share_intent.setType("*/*");//设置分享内容的类型
   /*     File file1 = new File(Environment.getExternalStoragePublicDirectory(
                getContext().getPackageName()), "log");
        File file = new File(file1, "/spvsdk.log");
        if (!file.exists()) {
            return;
        }*/
        share_intent.putExtra(Intent.EXTRA_STREAM, getImageContentUri(getContext(), curentFile));//添加分享内容
        startActivity(share_intent);
    }

    private static void checkFileUriExposure() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
            StrictMode.VmPolicy.Builder builder = new StrictMode.VmPolicy.Builder();
            StrictMode.setVmPolicy(builder.build());
            builder.detectFileUriExposure();
        }
    }

    public static Uri getImageContentUri(Context context, File imageFile) {
        String filePath = imageFile.getAbsolutePath();
        Cursor cursor = context.getContentResolver().query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                new String[]{MediaStore.Images.Media._ID}, MediaStore.Images.Media.DATA + "=? ",
                new String[]{filePath}, null);
        Uri uri = null;

        if (cursor != null) {
            if (cursor.moveToFirst()) {
                int id = cursor.getInt(cursor.getColumnIndex(MediaStore.MediaColumns._ID));
                Uri baseUri = Uri.parse("content://media/external/images/media");
                uri = Uri.withAppendedPath(baseUri, "" + id);
            }

            cursor.close();
        }

        if (uri == null) {
            ContentValues values = new ContentValues();
            values.put(MediaStore.Images.Media.DATA, filePath);
            uri = context.getContentResolver().insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values);
        }

        return uri;
    }
}
