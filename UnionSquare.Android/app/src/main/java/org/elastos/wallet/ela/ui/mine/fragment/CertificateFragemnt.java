package org.elastos.wallet.ela.ui.mine.fragment;

import android.hardware.fingerprint.FingerprintManager;
import android.os.Build;
import android.os.Bundle;
import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyProperties;
import android.support.annotation.RequiresApi;
import android.view.View;
import android.widget.Toast;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;

import java.security.KeyStore;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;

public class CertificateFragemnt extends BaseFragment {
    private KeyStore keyStore;
    private static final String DEFAULT_KEY_NAME = "default_key";
    private String type;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_certificate;
    }

    @Override
    protected void setExtraData(Bundle data) {
        type = data.getString("type");
    }

    @Override
    protected void initView(View view) {
        certificate();
    }

    private void certificate() {
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

    /**
     * 处理回退事件
     *
     * @return
     */
  /*  @Override
    public boolean onBackPressedSupport() {
        return closeApp();
    }*/
}
