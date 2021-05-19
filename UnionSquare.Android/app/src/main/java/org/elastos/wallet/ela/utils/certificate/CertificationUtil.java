package org.elastos.wallet.ela.utils.certificate;

import android.app.KeyguardManager;
import android.content.Context;
import android.content.Intent;
import android.hardware.fingerprint.FingerprintManager;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.RequiresApi;
import android.support.v4.app.Fragment;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.mine.fragment.CertificateFragemnt;
import org.elastos.wallet.ela.utils.SPUtil;


public class CertificationUtil {
    public static boolean fingerCertificating;//true 验证页面的打开状态 false 验证完成无论成功或者没验证返回
    public static int pwdCertificateStatus = 0;//由于密码验证结束后会出发onstart 定义状态  0 未开始  1正在验证  2 刚验证完不需要在在下面的start再次调用(验证成功失败都有可能)  3刚验证完成需要start再次调用(验证成功失败都有可能)

    @RequiresApi(api = Build.VERSION_CODES.M)
    public static boolean hasEnrolledFingerprints(Context context) {
        FingerprintManager fingerprintManager = null;
        try {
            fingerprintManager = (FingerprintManager) context.getSystemService(Context.FINGERPRINT_SERVICE);
        } catch (Throwable e) {
            //FPLog.log("have not class FingerprintManager");
        }
        return (fingerprintManager != null && fingerprintManager.isHardwareDetected() && fingerprintManager.hasEnrolledFingerprints());

    }

    public static boolean isKeyguardSecure(Context context) {
        KeyguardManager keyguardManager = null;
        try {
            keyguardManager = (KeyguardManager) context.getSystemService(Context.KEYGUARD_SERVICE);
        } catch (Throwable throwable) {
            // FPLog.log("getKeyguardManager exception");
        }
        return keyguardManager != null && keyguardManager.isKeyguardSecure();
    }

    public static KeyguardManager getKeyguardManager(Context context) {
        KeyguardManager keyguardManager = null;
        try {
            keyguardManager = (KeyguardManager) context.getSystemService(Context.KEYGUARD_SERVICE);
        } catch (Throwable throwable) {
            // FPLog.log("getKeyguardManager exception");
        }
        return keyguardManager;
    }

    public final static int REQUEST_CODE_CREDENTIALS_MINE = 10001;
    public final static int REQUEST_CODE_CREDENTIALS = 10002;

    public static void showAuthenticationScreen(Fragment fragment, KeyguardManager mKeyManager, int requestCode) {

        Intent intent = mKeyManager.createConfirmDeviceCredentialIntent(fragment.getString(R.string.lockpwd), "testlockscreenpwd");
        if (intent != null) {
            fragment.startActivityForResult(intent, requestCode);
        }
    }

    private static final String ACTION_SETTING = "android.settings.SETTINGS";

    public static void openFingerPrintSettingPage(Context context) {
        Intent intent = new Intent(ACTION_SETTING);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        try {
            context.startActivity(intent);
        } catch (Exception e) {
        }
    }

    public static void isOpenCertificate(BaseFragment fragment, int requstCode) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && hasEnrolledFingerprints(fragment.getContext())) {
            //指纹解锁
            fingerCertificating = true;
            Bundle bundle = new Bundle();
            bundle.putInt("requstCode", requstCode);
            if (requstCode == REQUEST_CODE_CREDENTIALS_MINE) {
                ((BaseFragment) (fragment.getParentFragment())).start(CertificateFragemnt.class, bundle);
            } else
                fragment.start(CertificateFragemnt.class, bundle);
        } else {
            //密码解锁
            KeyguardManager keyguardManager = getKeyguardManager(fragment.getContext());
            if (keyguardManager != null && keyguardManager.isKeyguardSecure()) {
                pwdCertificateStatus = 1;
                showAuthenticationScreen(fragment, keyguardManager, requstCode);
            } else {
                //设置页面
                new SPUtil(fragment.getContext()).setOpenCertificate(false);
                CertificationUtil.openFingerPrintSettingPage(fragment.getContext());
            }
        }
    }
}
