package org.elastos.wallet.ela.utils;


import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.content.FileProvider;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.BuildConfig;
import org.elastos.wallet.R;

import java.io.File;

/**
 * 在点击交互前进程已经被杀死  所有不能交互
 */
public class CrashDialog extends Activity {


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.dialog_apperro);
        CrashDialog.this.setFinishOnTouchOutside(false);
 /*       new Thread(){
            @Override
            public void run() {
                try {
                    sleep(2000);
                    exit();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }.start();*/



          initView();
    }

    /*private void sleep(long mills) {
        try {
            Thread.sleep(mills);
            finish();
            android.os.Process.killProcess(android.os.Process.myPid());
            System.exit(1);
        } catch (InterruptedException ex) {
            Log.e("ggg", "error : ", ex);
        }
    }*/

    ImageView ivCancel;
    TextView tvSure;
    TextView tvCancel;

    private void initView() {
        ivCancel = findViewById(R.id.iv_cancel);
        tvCancel = findViewById(R.id.tv_cancel);
        tvSure = findViewById(R.id.tv_sure);

        ivCancel.setOnClickListener(mOnClick);
        tvCancel.setOnClickListener(mOnClick);
        tvSure.setOnClickListener(mOnClick);

    }

    OnClickListener mOnClick = new OnClickListener() {

        @Override
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.iv_cancel:
                case R.id.tv_cancel:
                    exit();
                    break;
                case R.id.tv_sure:
                    File zipFile = new File(getExternalFilesDir("log"), "spvsdk.zip");
                    shareFile(zipFile.getAbsolutePath());
                    exit();
                    break;
                default:
                    break;
            }
        }
    };


    private void exit() {
        finish();
        android.os.Process.killProcess(android.os.Process.myPid());
        System.exit(1);
    }

    private void restart() {
        Intent intent = getBaseContext().getPackageManager()
                .getLaunchIntentForPackage(getBaseContext().getPackageName());
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
        exit();
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        exit();
    }

    private void shareFile(String date) {
        File file = new File(date);
        if (!file.exists()) {
            return;
        }
        Intent share_intent = new Intent();
        share_intent.setAction(Intent.ACTION_SEND);//设置分享行为
        share_intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        share_intent.setType("*/*");//设置分享内容的类型
        //  File file1 = getContext().getExternalFilesDir("log");
        Uri uri = getUri(this, BuildConfig.APPLICATION_ID + ".fileProvider", file);
        share_intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION
                | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
        share_intent.putExtra(Intent.EXTRA_STREAM, uri);//添加分享内容
        //startActivity(share_intent);
        this.startActivity(Intent.createChooser(share_intent, "share"));
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
