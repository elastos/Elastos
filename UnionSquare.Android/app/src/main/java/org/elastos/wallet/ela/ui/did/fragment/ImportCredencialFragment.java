package org.elastos.wallet.ela.ui.did.fragment;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.utils.FileChooseUtil;
import org.elastos.wallet.ela.utils.Log;

import java.io.File;

import butterknife.BindView;
import butterknife.OnClick;

public class ImportCredencialFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_title_right)
    TextView tvTitleRight;
    @BindView(R.id.rv)
    RecyclerView rv;
    private Wallet wallet;

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
    }


    @OnClick({R.id.tv_chose})
    public void onViewClicked(View view) {

        switch (view.getId()) {
            case R.id.tv_chose:
                requstManifestPermission(getString(R.string.needpermissionstorage));
                break;


        }
    }


    @Override
    protected void requstPermissionOk() {

        File dir = getBaseActivity().getExternalFilesDir("credentials" + File.separator + wallet.getWalletId());
        if (!dir.exists()) {
            dir.mkdirs();
        }
        //调用系统文件管理器打开指定路径目录
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setDataAndType(Uri.fromFile(dir.getParentFile()), "*.jwt");
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        startActivityForResult(intent, 0);

    }


    @Override
    public void onActivityResult(int requestCode,int resultCode,Intent data){//选择文件返回
        super.onActivityResult(requestCode,resultCode,data);
        if(resultCode==RESULT_OK){
            switch(requestCode){
                case 0:
                    Uri uri=data.getData();
                    String chooseFilePath = FileChooseUtil.getInstance(getContext()).getChooseFileResultPath(uri);
                    Log.d("??","选择文件返回："+chooseFilePath);
                    //sendFileMessage(chooseFilePath);
                    break;
            }
        }
    }





    private static final String[][] MIME_MapTable = {
            {".jwt", "text/plain"},
    };

    private String getMIMEType(File file) {

        String type = "*/*";
        String fName = file.getName();
        //获取后缀名前的分隔符"."在fName中的位置。
        int dotIndex = fName.lastIndexOf(".");
        if (dotIndex < 0)
            return type;
        /* 获取文件的后缀名 */
        String fileType = fName.substring(dotIndex, fName.length()).toLowerCase();
        if (fileType == null || "".equals(fileType))
            return type;
        //在MIME和文件类型的匹配表中找到对应的MIME类型。
        for (int i = 0; i < MIME_MapTable.length; i++) {
            if (fileType.equals(MIME_MapTable[i][0]))
                type = MIME_MapTable[i][1];
        }
        return type;
    }

    public void openAndroidFile(File file) {
        Intent intent = new Intent();
//        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);//设置标记
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        intent.setAction(Intent.ACTION_VIEW);//动作，查看
        intent.setDataAndType(Uri.fromFile(file), getMIMEType(file));//设置类型
        startActivity(intent);
    }

}
