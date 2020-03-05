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

public class ShowSocialAccountFragemnt /*extends BaseFragment*/ {

   /* @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.tv_pagehome)
    TextView tvPagehome;
    @BindView(R.id.tv_google)
    TextView tvGoogle;
    @BindView(R.id.tv_microsoft)
    TextView tvMicrosoft;
    @BindView(R.id.tv_facebook)
    TextView tvFacebook;
    @BindView(R.id.tv_twitter)
    TextView tvTwitter;
    @BindView(R.id.tv_webo)
    TextView tvWebo;
    @BindView(R.id.tv_wechat)
    TextView tvWechat;
    @BindView(R.id.tv_alipay)
    TextView tvAlipay;


    private CredentialSubjectBean info;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_socialaccount_show;
    }

    @Override
    protected void setExtraData(Bundle data) {
        info = data.getParcelable("CredentialSubjectBean");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.socialaccount);
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.found_vote_edit);
        setData();
    }

    private void setData() {
        CredentialSubjectBean.Social personal = info.getSocial();

        setText(personal.getHomePage(), tvPagehome);
        setText(personal.getGoogleAccount(), tvGoogle);
        setText(personal.getMicrosoftPassport(), tvMicrosoft);
        setText(personal.getFacebook(), tvFacebook);
        setText(personal.getTwitter(), tvTwitter);
        setText(personal.getWeibo(), tvWebo);
        setText(personal.getWechat(), tvWechat);
        setText(personal.getAlipay(), tvAlipay);

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
                start(SocialAccountFragment.class, bundle);
                break;

        }
    }
*/

}