package org.elastos.wallet.ela.ui.mine.fragment;

import android.os.Bundle;
import android.view.View;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.RxFingerPrinter;
import org.elastos.wallet.ela.utils.certificate.CertificationUtil;

import io.reactivex.observers.DisposableObserver;
import zwh.com.lib.FPerException;
import zwh.com.lib.IdentificationInfo;


public class CertificateFragemnt extends BaseFragment {

   /* private String type;
    private boolean certificating;*/
    private int requstCode;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_certificate;
    }

    @Override
    protected void setExtraData(Bundle data) {
       // type = data.getString("type");
        requstCode = data.getInt("requstCode");
    }

    @Override
    protected void initView(View view) {
        certificate();
    }

    //public String getDisplayMessage() {
//        switch (code) {
//            case SYSTEM_API_ERROR:
//                return "系统API小于23";
//            case PERMISSION_DENIED_ERROE:
//                return "没有指纹识别权限";
//            case HARDWARE_MISSIING_ERROR:
//                return "没有指纹识别模块";
//            case KEYGUARDSECURE_MISSIING_ERROR:
//                return "没有开启锁屏密码";
//            case NO_FINGERPRINTERS_ENROOLED_ERROR:
//                return "没有指纹录入";
//            case FINGERPRINTERS_FAILED_ERROR:
//                return "指纹认证失败，请稍后再试";
//            case FINGERPRINTERS_RECOGNIZE_FAILED:
//                return "指纹识别失败，请重试";
//            default:
//                return "";
//        }
//    }
    private void certificate() {
        RxFingerPrinter rxFingerPrinter = new RxFingerPrinter(getBaseActivity());
        // 可以在oncreat方法中执行
        DisposableObserver<IdentificationInfo> observer =
                new DisposableObserver<IdentificationInfo>() {
                    @Override
                    protected void onStart() {

                    }

                    @Override
                    public void onError(Throwable e) {

                    }

                    @Override
                    public void onComplete() {

                    }

                    @Override
                    public void onNext(IdentificationInfo info) {
                        if (info.isSuccessful()) {//识别成功
                            post(RxEnum.CERFICATION.ordinal(), null, requstCode);
                            pop();
                        } else {//识别失败 获取错误信息
                            FPerException exception = info.getException();
                            if (exception != null) {
                                //Toast.makeText(getContext(), exception.getDisplayMessage(), Toast.LENGTH_SHORT).show();
                            }
                        }
                    }
                };
        rxFingerPrinter.begin().subscribe(observer);//RxfingerPrinter会自动在onPause()时暂停指纹监听，onResume()时恢复指纹监听
    }

    /*private void certificate1() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            FingerprintManager.AuthenticationCallback callback = new FingerprintManager.AuthenticationCallback() {
                @Override
                public void onAuthenticationSucceeded(FingerprintManager.AuthenticationResult result) {
                    //指纹验证成功98
                    Log.d("????", "onAuthenticationSucceeded()");
                    if ("isOpenCertificate".equals(type)) {
                        post(RxEnum.CERFICATION.ordinal(), null, null);
                    }
                    popBackFragment();
                }

                @Override
                public void onAuthenticationError(int errorCode, CharSequence errString) {
                    //指纹验证失败，不可再验
                    Log.d("????", "onAuthenticationError()" + errString);
                }

                @Override
                public void onAuthenticationHelp(int helpCode, CharSequence helpString) {
                    //指纹验证失败，可再验，可能手指过脏，或者移动过快等原因。
                    if (helpCode == 1021 || helpCode == 1022 || helpCode == 1023 || helpCode == 1001) {

                    }
                    Log.d("????", "onAuthenticationHelp()" + helpCode + "//" + helpString);
                }

                @Override
                public void onAuthenticationFailed() {
                    //指纹验证失败，指纹识别失败，可再验，该指纹不是系统录入的指纹。
                    Log.d("????", "onAuthenticationFailed():不能识别");
                }
            };


            FingerprintManager fingerprintManager = getActivity().getSystemService(FingerprintManager.class);
            if (fingerprintManager.isHardwareDetected() && fingerprintManager.hasEnrolledFingerprints()) {
                fingerprintManager.authenticate(new FingerprintManager.CryptoObject(initCipher()), null, 0, callback, null);
            } else {
                Toast.makeText(getActivity(), "onAuthenticationFailed():不支持", Toast.LENGTH_SHORT).show();
            }
        }
    }


    // 初始化密钥库
    @RequiresApi(api = Build.VERSION_CODES.M)
    private void initKey() {
        try {
            keyStore = KeyStore.getInstance("AndroidKeyStore");
            keyStore.load(null);
            KeyGenerator keyGenerator = KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_AES, "AndroidKeyStore");
            KeyGenParameterSpec.Builder builder = new KeyGenParameterSpec.Builder(DEFAULT_KEY_NAME,
                    KeyProperties.PURPOSE_ENCRYPT |
                            KeyProperties.PURPOSE_DECRYPT)
                    .setBlockModes(KeyProperties.BLOCK_MODE_CBC)
                    .setUserAuthenticationRequired(true)
                    .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_PKCS7);
            keyGenerator.init(builder.build());
            keyGenerator.generateKey();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    // 初始化密钥
    @RequiresApi(api = Build.VERSION_CODES.M)
    private Cipher initCipher() {

        try {
            initKey();
            SecretKey key = (SecretKey) keyStore.getKey(DEFAULT_KEY_NAME, null);
            Cipher cipher = Cipher.getInstance(KeyProperties.KEY_ALGORITHM_AES + "/"
                    + KeyProperties.BLOCK_MODE_CBC + "/"
                    + KeyProperties.ENCRYPTION_PADDING_PKCS7);
            cipher.init(Cipher.ENCRYPT_MODE, key);
            return cipher;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }
*/

    /**
     * 处理回退事件
     *
     * @return
     */
    @Override
    public boolean onBackPressedSupport() {
        if (requstCode== CertificationUtil.REQUEST_CODE_CREDENTIALS_MINE){
            return super.onBackPressedSupport();
        }
        return closeApp();
    }
}
