package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DateUtil;

import butterknife.BindView;
import butterknife.ButterKnife;
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
        if (info.getInfo() != null) {
            tvPersonlinfoNo.setVisibility(View.GONE);
            tvPersonlinfoTime.setVisibility(View.VISIBLE);
            tvPersonlinfoTime.setText(getString(R.string.keeptime) + DateUtil.time(info.getInfo().getEditTime()));

        }
        if (info.getIntro() != null) {
            tvPersonlintroNo.setVisibility(View.GONE);
            tvPersonlinfoTime.setVisibility(View.VISIBLE);
            tvPersonlintroTime.setText(getString(R.string.keeptime) + DateUtil.time(info.getIntro().getEditTime()));

        }
        if (info.getSocial() != null) {
            tvSocialNo.setVisibility(View.GONE);
            tvPersonlinfoTime.setVisibility(View.VISIBLE);
            tvSocialTime.setText(getString(R.string.keeptime) + DateUtil.time(info.getSocial().getEditTime()));

        }

    }

    @OnClick({R.id.ll_personalinfo, R.id.ll_personalintro, R.id.ll_social, R.id.tv_out, R.id.tv_in})
    public void onViewClicked(View view) {
        Bundle bundle = new Bundle();
        bundle.putParcelable("CredentialSubjectBean", info);
        switch (view.getId()) {
            case R.id.ll_personalinfo:
                if (info.getInfo() != null) {
                    start(ShowPersonalInfoFragemnt.class, bundle);
                } else {
                    bundle.putString("type", Constant.ADDPERSONALINFO);
                    start(PersonalInfoFragment.class, bundle);
                }
                break;
            case R.id.ll_personalintro:
                break;
            case R.id.ll_social:
                break;
            case R.id.tv_out:

                break;
            case R.id.tv_in:

                break;
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        // TODO: inflate a fragment view
        View rootView = super.onCreateView(inflater, container, savedInstanceState);
        unbinder1 = ButterKnife.bind(this, rootView);
        return rootView;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder1.unbind();
    }
}
