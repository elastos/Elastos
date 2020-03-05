package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.view.View;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.bean.CallBackJwtEntity;
import org.elastos.wallet.ela.ui.Assets.bean.RecieveJwtEntity;
import org.elastos.wallet.ela.ui.did.presenter.AuthorizationPresenter;
import org.elastos.wallet.ela.utils.Log;

import butterknife.OnClick;

public class AuthorizationFragment extends BaseFragment implements NewBaseViewData {
    @Override
    protected int getLayoutId() {
        return R.layout.fragment_did_authorization;
    }

    RecieveJwtEntity recieveJwtEntity;
    String callBackUrl;
    @Override
    protected void setExtraData(Bundle data) {
        recieveJwtEntity = data.getParcelable("RecieveJwtEntity");
    }

    @Override
    protected void initView(View view) {
         callBackUrl = recieveJwtEntity.getCallbackurl();


    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "postData":
              //  Log.d("??", "111");
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
                CallBackJwtEntity callBackJwtEntity=new CallBackJwtEntity();
                callBackJwtEntity.setType("credaccess");
                callBackJwtEntity.setIss("credaccess");

                new AuthorizationPresenter().postData(callBackUrl, "111", this);
                break;
            case R.id.tv_refuse:

                popBackFragment();
                break;


        }
    }
}
