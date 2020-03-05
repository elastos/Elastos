package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.utils.Constant;

import butterknife.BindView;
import butterknife.OnClick;

public class ShowPersonalIntroFragemnt extends BaseFragment {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.tv_intro)
    TextView tvIntro;


    private CredentialSubjectBean info;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_personalintro_show;
    }

    @Override
    protected void setExtraData(Bundle data) {
        info = data.getParcelable("CredentialSubjectBean");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.personalintro));
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.found_vote_edit);
        setData();
    }

    private void setData() {
        /*CredentialSubjectBean.Intro personal = info.getIntro();

        setText(personal.getIntroduction(), tvIntro);*/

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
                start(PersonalIntroFragment.class, bundle);
                break;

        }
    }


}