package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.view.View;

import com.alibaba.fastjson.JSON;

import org.elastos.did.exception.DIDStoreException;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.bean.CallBackJwtEntity;
import org.elastos.wallet.ela.ui.Assets.bean.RecieveJwtEntity;
import org.elastos.wallet.ela.ui.did.presenter.AuthorizationPresenter;

import java.util.Date;

import butterknife.OnClick;

public class AuthorizationFragment extends BaseFragment implements NewBaseViewData {
    private String[] jwtParts;
    RecieveJwtEntity recieveJwtEntity;
    private String scanResult;
    private String payPasswd;
    String callBackUrl;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_did_authorization;
    }


    @Override
    protected void setExtraData(Bundle data) {
        scanResult = data.getString("scanResult");
        payPasswd = data.getString("payPasswd");
        String result = scanResult.replace("elastos://credaccess/", "");
        jwtParts = result.split("\\.");
        String payload = new String(org.elastos.did.util.Base64.decode(jwtParts[1]));
        recieveJwtEntity = JSON.parseObject(payload, RecieveJwtEntity.class);
    }

    @Override
    protected void initView(View view) {
        callBackUrl = recieveJwtEntity.getCallbackurl();


    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "postData":
              showToast("授权成功");
                popBackFragment();
                break;
        }
    }

    @OnClick({R.id.tv_agree, R.id.tv_refuse})
    public void onViewClicked(View view) {
        Bundle bundle;
        switch (view.getId()) {
            case R.id.tv_agree:
                //{
                //"type":”credaccess",
                //"iss":"授权的did",
                //"iat": "二维码生成时间",
                //"exp": "二维码有效期",
                //"aud":"网站did",
                //"req": "网站二维码包含的内容",
                //"presentation":"授权的数据内容"
                //}
                CallBackJwtEntity callBackJwtEntity = new CallBackJwtEntity();
                callBackJwtEntity.setType("credaccess");
                callBackJwtEntity.setIss(getMyDID().getDidString());
                callBackJwtEntity.setIat(new Date().getTime());
                callBackJwtEntity.setExp(new Date().getTime() + 5 * 60 * 1000);
                callBackJwtEntity.setAud(recieveJwtEntity.getIss());
                callBackJwtEntity.setReq(scanResult);
                callBackJwtEntity.setPresentation("");
                callBackJwtEntity.setUserId(recieveJwtEntity.getUserId());

                String header = jwtParts[0];
                String payload = org.elastos.did.util.Base64.encodeToString(JSON.toJSONString(callBackJwtEntity).getBytes());
                try {
                    String signature = getMyDID().getDIDDocument().sign(payPasswd, (header + "." + payload).getBytes());
                    new AuthorizationPresenter().postData(callBackUrl, header + "." + payload + "." + signature, this);
                } catch (DIDStoreException e) {
                    e.printStackTrace();
                }

                break;
            case R.id.tv_refuse:

                popBackFragment();
                break;


        }
    }
}
