package org.elastos.wallet.ela.utils.IBiometricPrompt;

import android.app.Activity;
import android.app.KeyguardManager;
import android.hardware.fingerprint.FingerprintManager;
import android.os.Build;
import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyProperties;
import android.support.annotation.RequiresApi;
import android.widget.Toast;

import java.security.KeyStore;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;

public class IBiometricPromptlManager {
    Activity activity;
    private KeyStore keyStore;
    private static final String DEFAULT_KEY_NAME = "default_key";
    private Cipher mCipher;

    public IBiometricPromptlManager( Activity activity){
        this.activity=activity;

    }
    public boolean supportFingerprint() {
        if (Build.VERSION.SDK_INT < 23) {
            Toast.makeText(activity, "您的系统版本过低，不支持指纹功能", Toast.LENGTH_SHORT).show();
            return false;
        } else {
            KeyguardManager keyguardManager = activity.getSystemService(KeyguardManager.class);
            FingerprintManager fingerprintManager = activity.getSystemService(FingerprintManager.class);
            if (fingerprintManager != null) {
                if (!fingerprintManager.isHardwareDetected()) {
                    Toast.makeText(activity, "您的手机不支持指纹功能", Toast.LENGTH_SHORT).show();
                    return false;
                } else if (keyguardManager != null) {
                    if (!keyguardManager.isKeyguardSecure()) {
                        Toast.makeText(activity, "您还未设置锁屏，请先设置锁屏并添加一个指纹", Toast.LENGTH_SHORT).show();
                        return false;
                    } else if (!fingerprintManager.hasEnrolledFingerprints()) {
                        Toast.makeText(activity, "您至少需要在系统设置中添加一个指纹", Toast.LENGTH_SHORT).show();
                        return false;
                    }
                } else {
                    Toast.makeText(activity, "键盘管理初始化失败", Toast.LENGTH_SHORT).show();
                    return false;
                }
            } else {
                Toast.makeText(activity, "指纹管理初始化失败", Toast.LENGTH_SHORT).show();
                return false;
            }
        }
        return true;
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
    private void initCipher() {
        try {
            SecretKey key = (SecretKey) keyStore.getKey(DEFAULT_KEY_NAME, null);
            Cipher cipher = Cipher.getInstance(KeyProperties.KEY_ALGORITHM_AES + "/"
                    + KeyProperties.BLOCK_MODE_CBC + "/"
                    + KeyProperties.ENCRYPTION_PADDING_PKCS7);
            cipher.init(Cipher.ENCRYPT_MODE, key);
            setCipher(cipher);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }
    public void setCipher(Cipher cipher) {
        mCipher = cipher;
    }
}
