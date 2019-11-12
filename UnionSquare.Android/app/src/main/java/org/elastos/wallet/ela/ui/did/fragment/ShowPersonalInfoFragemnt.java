package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.GlideApp;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class ShowPersonalInfoFragemnt extends BaseFragment {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_icon)
    ImageView ivIcon;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.tv_name)
    TextView tvName;
    @BindView(R.id.tv_nick)
    TextView tvNick;
    @BindView(R.id.tv_sex)
    TextView tvSex;
    @BindView(R.id.tv_birthday)
    TextView tvBirthday;
    @BindView(R.id.tv_email)
    TextView tvEmail;
    @BindView(R.id.tv_phonenumber)
    TextView tvPhonenumber;
    @BindView(R.id.tv_area)
    TextView tvArea;
    Unbinder unbinder;


    private CredentialSubjectBean info;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_personalinfo_show;
    }

    @Override
    protected void setExtraData(Bundle data) {
        info = data.getParcelable("CredentialSubjectBean");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.personalindo);
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.found_vote_edit);
        setData();
    }

    private void setData() {
        CredentialSubjectBean.Info personal = info.getInfo();
        GlideApp.with(this).load(personal.getAvatar())
                .error(R.mipmap.mine_did_default_avator).circleCrop().into(ivIcon);
        setText(personal.getName(), tvName);
        setText(personal.getNickname(), tvNick);
        setText(personal.getGender(), tvSex);
        setText(DateUtil.timeNYR(personal.getBirthday(), getContext()), tvBirthday);
        setText(personal.getEmail(), tvEmail);
        String phone = null;
        if (!TextUtils.isEmpty(personal.getPhoneCode()) && !TextUtils.isEmpty(personal.getPhone())) {
            phone = "+" + personal.getPhoneCode() + personal.getPhone();

        } else if (TextUtils.isEmpty(personal.getPhoneCode())) {
            phone = personal.getPhone();
        } else if (TextUtils.isEmpty(personal.getPhone())) {
            phone = "+" + personal.getPhoneCode();
        }

        setText(phone, tvPhonenumber);
        setText(AppUtlis.getLoc(getContext(), personal.getNation()), tvArea);
    }

    private void setText(String text, TextView textView) {
        if (TextUtils.isEmpty(text)) {
            ((ViewGroup) (textView.getParent())).setVisibility(View.GONE);
        } else {
            textView.setText(text);
        }

    }

    @OnClick({R.id.iv_title_right})
    public void onViewClicked(View view) {
        Bundle bundle = new Bundle();
        bundle.putParcelable("CredentialSubjectBean", info);
        switch (view.getId()) {
            case R.id.iv_title_right:
                bundle.putString("type", Constant.EDITCREDENTIAL);
                start(PersonalInfoFragment.class, bundle);
                break;

        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        // TODO: inflate a fragment view
        View rootView = super.onCreateView(inflater, container, savedInstanceState);
        unbinder = ButterKnife.bind(this, rootView);
        return rootView;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }
}