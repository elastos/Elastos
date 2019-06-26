package org.elastos.wallet.ela.ui.mine.fragment;

import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.os.StrictMode;
import android.provider.MediaStore;
import android.text.TextUtils;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.mine.presenter.LogFilePresenter;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.MyUtil;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.ScanQRcodeUtil;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.prefs.AbstractPreferences;

import butterknife.BindView;
import butterknife.OnClick;

public class AboutFragment extends BaseFragment implements CommmonStringWithMethNameViewData {

    @BindView(R.id.tv_title)
    TextView tvTitle;

    @BindView(R.id.tv_version_name)
    TextView tvVersionName;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_about;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.about));
        tvVersionName.setText(getString(R.string.currentversionname) + MyUtil.getVersionName(getContext()));

    }


    @OnClick({R.id.tv_updatalog, R.id.tv_feedback, R.id.tv_runlog})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_updatalog:
                Intent intent = new Intent("android.intent.action.VIEW");
                intent.setData(Uri.parse(Constant.UpdateLog + "?langua =" + (new SPUtil(getContext()).getLanguage() == 0 ? "ch" : "en")));
                startActivity(intent);
                break;
            case R.id.tv_feedback:
                ClipboardUtil.copyClipboar(getBaseActivity(), Constant.Email, getResources().getString(R.string.copyemailsuccess));
                break;
            case R.id.tv_runlog:
                //导出c运行日志
                requstManifestPermission(getString(R.string.needpermissionstorage));
                //shareFile();
                break;
        }
    }


    @Override
    protected void requstPermissionOk() {
        new LogFilePresenter().moveLogFile(this);
    }

    private void shareFile() {
        checkFileUriExposure();
        Intent share_intent = new Intent();
        share_intent.setAction(Intent.ACTION_SEND);//设置分享行为
        share_intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        share_intent.setType("*/*");//设置分享内容的类型
        File file1 = new File(Environment.getExternalStoragePublicDirectory(
                getContext().getPackageName()), "log");
        File file = new File(file1, "/spvsdk.log");
        if (!file.exists()) {
            return;
        }
        share_intent.putExtra(Intent.EXTRA_STREAM, getImageContentUri(getContext(), file));//添加分享内容
        startActivity(share_intent);
    }

    @Override
    public void onGetCommonData(String methodname, String data) {

        if (!TextUtils.isEmpty(data)) {
            showToast(getContext().getString(R.string.logkeppin) + data);
            shareFile();
        } else {
            showToast(getContext().getString(R.string.logkeepfail));
        }
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
