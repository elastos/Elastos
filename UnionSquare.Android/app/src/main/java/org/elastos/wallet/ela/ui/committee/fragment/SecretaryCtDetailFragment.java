package org.elastos.wallet.ela.ui.committee.fragment;

import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.committee.bean.CtDetailBean;
import org.elastos.wallet.ela.ui.committee.presenter.GeneralDetailPresenter;
import org.elastos.wallet.ela.ui.did.fragment.AuthorizationFragment;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.svg.GlideApp;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * secretary general detail fragment
 */
public class SecretaryCtDetailFragment extends BaseFragment implements NewBaseViewData {

    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    GeneralDetailPresenter presenter;

    private String id;
    private String did;
    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        id = data.getString("id");
        did = data.getString("did");
    }

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_secretary_detail;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getContext().getString(R.string.secretarydetail));
        presenter = new GeneralDetailPresenter();
        presenter.getCouncilInfo(this, id, did);
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getCouncilInfo":
                setInfo((CtDetailBean) baseEntity);
                break;
        }
    }

    @BindView(R.id.head_ic)
    ImageView head;
    @BindView(R.id.name)
    TextView name;
    @BindView(R.id.location)
    TextView location;
    @BindView(R.id.secretary_did)
    TextView didTv;
    @BindView(R.id.end_time)
    TextView endTime;
    @BindView(R.id.from_time)
    TextView fromTime;
    @BindView(R.id.birth_time)
    TextView birthDay;
    @BindView(R.id.email)
    TextView email;
    @BindView(R.id.personal_homepage)
    TextView homepage;
    @BindView(R.id.wechat_account)
    TextView wechat;
    @BindView(R.id.blog_account)
    TextView weibo;
    @BindView(R.id.facebook_account)
    TextView facebook;
    @BindView(R.id.microsoft_account)
    TextView microsoft;
    @BindView(R.id.personal_profile)
    TextView introduce;
    private void setInfo(CtDetailBean ctDetailBean) {
        CtDetailBean.DataBean dataBean = ctDetailBean.getData().get(0);
        GlideApp.with(getContext()).load(dataBean.getAvatar()).error(R.mipmap.icon_ela).circleCrop().into(head);
        name.setText(dataBean.getDidName());
        location.setText(AppUtlis.getLoc(getContext(), String.valueOf(dataBean.getLocation())));
        didTv.setText(dataBean.getDid());
        endTime.setText(DateUtil.formatTimestamp(String.valueOf(dataBean.getEndDate()), "yyyy.MM.dd"));
        fromTime.setText(DateUtil.formatTimestamp(String.valueOf(dataBean.getStartDate()), "yyyy.MM.dd"));
        birthDay.setText(DateUtil.formatTimestamp(String.valueOf(dataBean.getBirthday()), "yyyy.MM.dd"));
        email.setText(dataBean.getEmail());
        homepage.setText(dataBean.getAddress());
        wechat.setText(dataBean.getWechat());
        weibo.setText(dataBean.getWeibo());
        facebook.setText(dataBean.getFacebook());
        microsoft.setText(dataBean.getMicrosoft());
        introduce.setText(dataBean.getIntroduction());
    }

    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();

    @OnClick({R.id.refresh_ct_did})
    public void onClick(View view) {
        Bundle bundle = new Bundle();
        bundle.putString("type", "authorization");
        bundle.putParcelable("wallet", wallet);
        start(AuthorizationFragment.class, bundle);
    }

}
