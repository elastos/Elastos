package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class CredentialFragment extends BaseFragment {


    @BindView(R.id.tv_personlinfo_time)
    TextView tvPersonlinfoTime;
    @BindView(R.id.tv_personlinfo_no)
    TextView tvPersonlinfoNo;
    @BindView(R.id.tv_personlintro_time)
    TextView tvPersonlintroTime;
    @BindView(R.id.tv_personlintro_no)
    TextView tvPersonlintroNo;
    @BindView(R.id.tv_social_time)
    TextView tvSocialTime;
    @BindView(R.id.tv_social_no)
    TextView tvSocialNo;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    Unbinder unbinder1;
    private CredentialSubjectBean info;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_did_credential;
    }

    @Override
    protected void setExtraData(Bundle data) {
        String did = data.getString("did");
        info = CacheUtil.getCredentialSubjectBean(did);
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.credentialinfo);
        viewInfo();
        viewIntro();
        viewSocial();


        registReceiver();

    }

    private void viewInfo() {
       /* if (info.getInfo() != null) {
            tvPersonlinfoNo.setVisibility(View.GONE);
            tvPersonlinfoTime.setVisibility(View.VISIBLE);
            tvPersonlinfoTime.setText(getString(R.string.keeptime) + DateUtil.time(info.getInfo().getEditTime()));

        } else {
            tvPersonlinfoNo.setVisibility(View.VISIBLE);
            tvPersonlinfoTime.setVisibility(View.GONE);
        }*/
    }

    private void viewIntro() {
      /*  if (info.getIntro() != null) {
            tvPersonlintroNo.setVisibility(View.GONE);
            tvPersonlintroTime.setVisibility(View.VISIBLE);
            tvPersonlintroTime.setText(getString(R.string.keeptime) + DateUtil.time(info.getIntro().getEditTime()));

        } else {
            tvPersonlintroNo.setVisibility(View.VISIBLE);
            tvPersonlintroTime.setVisibility(View.GONE);
        }*/
    }

    private void viewSocial() {
        /*if (info.getSocial() != null) {
            tvSocialNo.setVisibility(View.GONE);
            tvSocialTime.setVisibility(View.VISIBLE);
            tvSocialTime.setText(getString(R.string.keeptime) + DateUtil.time(info.getSocial().getEditTime()));

        } else {
            tvSocialNo.setVisibility(View.VISIBLE);
            tvSocialTime.setVisibility(View.GONE);
        }*/
    }

    @OnClick({R.id.ll_personalinfo, R.id.ll_personalintro, R.id.ll_social, R.id.tv_out, R.id.tv_in})
    public void onViewClicked(View view) {
        Bundle bundle = new Bundle();
        bundle.putParcelable("CredentialSubjectBean", info);
        switch (view.getId()) {
            case R.id.ll_personalinfo:
               /* if (info.getInfo() != null) {
                    start(ShowPersonalInfoFragemnt.class, bundle);
                } else {
                    bundle.putString("type", Constant.ADDCREDENTIAL);
                    start(PersonalInfoFragment.class, bundle);
                }*/
                break;
            case R.id.ll_personalintro:
              /*  if (info.getIntro() != null) {
                    start(ShowPersonalIntroFragemnt.class, bundle);
                } else {
                    bundle.putString("type", Constant.ADDCREDENTIAL);
                    start(PersonalIntroFragment.class, bundle);
                }*/
                break;
            case R.id.ll_social:
               /* if (info.getSocial() != null) {
                    start(ShowSocialAccountFragemnt.class, bundle);
                } else {
                    bundle.putString("type", Constant.ADDCREDENTIAL);
                    start(SocialAccountFragment.class, bundle);
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
        if (integer == RxEnum.EDITPERSONALINTRO.ordinal()) {
            viewIntro();

        }
        if (integer == RxEnum.EDITSOCIAL.ordinal()) {
            viewSocial();

        }


    }
}
