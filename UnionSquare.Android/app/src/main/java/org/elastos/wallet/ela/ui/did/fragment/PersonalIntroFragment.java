package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.entity.DIDInfoEntity;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.RxEnum;

import java.util.Date;

import butterknife.BindView;
import butterknife.OnClick;

public class PersonalIntroFragment extends BaseFragment {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_title_right)
    TextView tvTitleRight;
    @BindView(R.id.et_intro)
    EditText etIntro;
    @BindView(R.id.tv_next)
    TextView tvNext;
    @BindView(R.id.tv_tip)
    TextView tvTip;
    private DIDInfoEntity didInfo;
    private CredentialSubjectBean credentialSubjectBean;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_personalintro;
    }

    @Override
    protected void setExtraData(Bundle data) {
        String type = data.getString("type");
        if (Constant.EDITCREDENTIAL.equals(type)) {
            //编辑did  从凭证信息进入
            onAddPartCredential(data);
            putData();

        } else if (Constant.ADDCREDENTIAL.equals(type)) {
            //新增did  从凭证信息进入
            onAddPartCredential(data);
        } else {
            tvTitleRight.setVisibility(View.VISIBLE);
            tvTitle.setText(getString(R.string.addpersonalintro));
            didInfo = data.getParcelable("didInfo");
            credentialSubjectBean = data.getParcelable("credentialSubjectBean");
            if (data.getBoolean("useDraft"))
                putData();
        }
    }

    private void onAddPartCredential(Bundle data) {
        credentialSubjectBean = data.getParcelable("CredentialSubjectBean");
        tvTitle.setText(getString(R.string.editpersonalintro));
        tvTip.setVisibility(View.GONE);
        tvTitleRight.setVisibility(View.GONE);
        tvNext.setText(getString(R.string.keep));
        tvNext.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setData();
                CacheUtil.setCredentialSubjectBean(credentialSubjectBean);
                post(RxEnum.EDITPERSONALINTRO.ordinal(), null, null);
                popTo(CredentialFragment.class, false);
            }
        });
    }

    private void putData() {
        CredentialSubjectBean.Intro intro = credentialSubjectBean.getIntro();
        if (intro != null)
            etIntro.setText(intro.getIntroduction());
    }


    @Override
    protected void initView(View view) {


    }


    @OnClick({R.id.tv_next, R.id.tv_title_right})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_title_right:
            case R.id.tv_next:
                setData();
                start(SocialAccountFragment.class, getArguments());
                break;


        }
    }

    private void setData() {
        CredentialSubjectBean.Intro intro = credentialSubjectBean.getIntro();
        if (getText(etIntro) != null) {
            if (intro == null) {
                intro = new CredentialSubjectBean.Intro();
                credentialSubjectBean.setIntro(intro);
            }
            intro.setIntroduction(getText(etIntro));
            intro.setEditTime(new Date().getTime() / 1000);
        } else {
            credentialSubjectBean.setIntro(null);
        }
    }

    @Override
    public boolean onBackPressedSupport() {
        setData();
        return super.onBackPressedSupport();


    }
}
