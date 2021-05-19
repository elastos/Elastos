package org.elastos.wallet.ela.ui.mine.fragment;

import android.hardware.fingerprint.FingerprintManager;
import android.os.Build;
import android.os.Bundle;
import android.os.CancellationSignal;
import android.support.annotation.RequiresApi;
import android.view.View;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.certificate.CertificationUtil;


@RequiresApi(api = Build.VERSION_CODES.M)
public class CertificateFragemnt extends BaseFragment {

    private int requstCode;
    private FingerprintManager fingerprintManager;
    private FingerprintManager.AuthenticationCallback callback;
    private CancellationSignal mCancellationSignal;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_certificate;
    }

    @Override
    protected void setExtraData(Bundle data) {
        requstCode = data.getInt("requstCode");
    }

    @Override
    protected void initView(View view) {
        fingerprintManager = getActivity().getSystemService(FingerprintManager.class);
        intCallBack();

    }

    private void intCallBack() {
        callback = new FingerprintManager.AuthenticationCallback() {
            @Override
            public void onAuthenticationSucceeded(FingerprintManager.AuthenticationResult result) {
                //指纹验证成功98
                Log.d("????", "onAuthenticationSucceeded()");

                if (requstCode == CertificationUtil.REQUEST_CODE_CREDENTIALS_MINE) {
                    post(RxEnum.CERFICATION.ordinal(), null, requstCode);//通知mine操作
                }

                pop();
            }

            @Override
            public void onAuthenticationError(int errorCode, CharSequence errString) {
                //指纹验证失败，不可再验  在取消或者按home键也会触发这里
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
    }

    private void authenticate() {
        mCancellationSignal = new CancellationSignal();
        fingerprintManager.authenticate(null, mCancellationSignal, 0, callback, null);
    }


    private void cancel() {
        if (mCancellationSignal != null && !mCancellationSignal.isCanceled()) {
            mCancellationSignal.cancel();
            mCancellationSignal = null;
        }
    }


    public void onResume() {
        super.onResume();
        authenticate();
        Log.d("???", "onResume");

    }

    @Override
    public void onPause() {
        super.onPause();
        cancel();
        Log.d("???", "onPause");
    }


    @Override
    public void onDestroy() {
        super.onDestroy();
        cancel();
        fingerprintManager = null;
        callback = null;
        CertificationUtil.fingerCertificating = false;
    }

    /**
     * 处理回退事件
     *
     * @return
     */
    @Override
    public boolean onBackPressedSupport() {
        if (requstCode == CertificationUtil.REQUEST_CODE_CREDENTIALS_MINE) {
            return super.onBackPressedSupport();
        }
        return closeApp();
    }
}
