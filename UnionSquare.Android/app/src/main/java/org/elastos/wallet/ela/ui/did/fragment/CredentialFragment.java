package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.OnClick;

public class CredentialFragment extends BaseFragment {


    @BindView(R.id.tv_personlinfo_time)
    TextView tvPersonlinfoTime;
    @BindView(R.id.tv_personlinfo_no)
    TextView tvPersonlinfoNo;

    @BindView(R.id.tv_title)
    TextView tvTitle;

    private CredentialSubjectBean info;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_did_credential;
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.credentialinfo);
        viewInfo();
        registReceiver();

    }

    private void viewInfo() {
        String json = getMyDID().getCredentialJSon(getMyDID().getDidString());
        info = JSON.parseObject(json, CredentialSubjectBean.class);
       // Log.i("??",  (JSON.parseObject("{}", CredentialSubjectBean.class) == null));//false
        if (info != null) {
            tvPersonlinfoNo.setVisibility(View.GONE);
            tvPersonlinfoTime.setVisibility(View.VISIBLE);
            tvPersonlinfoTime.setText(getString(R.string.keeptime) + DateUtil.timeNYR(info.getEditTime(), getContext(), false));

        } else {
            tvPersonlinfoNo.setVisibility(View.VISIBLE);
            tvPersonlinfoTime.setVisibility(View.GONE);
        }
    }


    @OnClick({R.id.ll_personalinfo, R.id.tv_out, R.id.tv_in})
    public void onViewClicked(View view) {
        Bundle bundle = new Bundle();
        bundle.putParcelable("CredentialSubjectBean", info);
        switch (view.getId()) {
            case R.id.ll_personalinfo:
              /*  if (info != null) {
                    start(ShowPersonalInfoFragemnt.class, bundle);
                } else {
                    bundle.putString("type", Constant.ADDCREDENTIAL);
                    start(PersonalInfoFragment.class, bundle);
                }*/
                break;

            case R.id.tv_out:

                break;
            case R.id.tv_in:

                break;
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.EDITPERSONALINFO.ordinal()) {
            viewInfo();

        }


    }
}
