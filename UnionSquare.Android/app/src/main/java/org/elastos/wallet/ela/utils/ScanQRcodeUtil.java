package org.elastos.wallet.ela.utils;

import android.content.Intent;

import org.elastos.wallet.ela.base.BaseFragment;

import javax.inject.Inject;

import io.github.xudaojie.qrcodelib.CaptureActivity;

/**
 * Created by wangdongfeng on 2018/4/26.
 */

public class ScanQRcodeUtil {

    @Inject
    public ScanQRcodeUtil() {
    }



    public static final int SCAN_QR_REQUEST_CODE = 20080;

    public void scanQRcode(BaseFragment activity) {
        Intent intent = new Intent(activity.getContext(), CaptureActivity.class);
        activity.startActivityForResult(intent, SCAN_QR_REQUEST_CODE);
    }
}
