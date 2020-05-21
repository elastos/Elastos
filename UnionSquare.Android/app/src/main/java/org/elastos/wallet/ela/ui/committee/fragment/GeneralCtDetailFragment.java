package org.elastos.wallet.ela.ui.committee.fragment;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.support.v7.widget.AppCompatImageView;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.gson.Gson;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.committee.adaper.CtExpRecAdapter;
import org.elastos.wallet.ela.ui.committee.bean.CtDetailBean;
import org.elastos.wallet.ela.ui.committee.bean.ExperienceBean;
import org.elastos.wallet.ela.ui.committee.presenter.CtDetailPresenter;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.svg.GlideApp;
import org.elastos.wallet.ela.utils.view.CircleProgressView;

import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * general committee detail
 */
public class GeneralCtDetailFragment extends BaseFragment implements NewBaseViewData {
    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.ct_tv)
    TextView ctTitleTv;
    @BindView(R.id.line1)
    View line1;
    @BindView(R.id.experience_tv)
    TextView experienceTitleTv;
    @BindView(R.id.line2)
    View line2;
    @BindView(R.id.personal_info)
    View personalInfo;
    @BindView(R.id.work_experience)
    View workExperience;
    @BindView(R.id.recyclerview)
    RecyclerView recyclerView;
    @BindView(R.id.no_info)
    TextView norecord;
    CtExpRecAdapter adapter;
    List<CtDetailBean.Term> list = null;

    CtDetailPresenter presenter;

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
        return R.layout.fragment_ct_general_detail;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getContext().getString(R.string.ctdetail));
        presenter = new CtDetailPresenter();
        presenter.getCouncilInfo(this, id, did);
        experienceTitleTv.setText(String.format(getString(R.string.performancerecord), String.valueOf(0)));
    }

    private void selectDetail() {
        line1.setVisibility(View.VISIBLE);
        line2.setVisibility(View.GONE);
        ctTitleTv.setTextColor(getResources().getColor(R.color.whiter));
        experienceTitleTv.setTextColor(getResources().getColor(R.color.whiter50));
        personalInfo.setVisibility(View.VISIBLE);
        workExperience.setVisibility(View.GONE);
    }

    private void selectExperience() {
        line1.setVisibility(View.GONE);
        line2.setVisibility(View.VISIBLE);
        ctTitleTv.setTextColor(getResources().getColor(R.color.whiter50));
        experienceTitleTv.setTextColor(getResources().getColor(R.color.whiter));
        personalInfo.setVisibility(View.GONE);
        workExperience.setVisibility(View.VISIBLE);
    }

    private void setRecyclerView(List<CtDetailBean.Term> datas) {
        if(datas==null || datas.size()<=0) return;
        norecord.setVisibility(View.GONE);
        recyclerView.setVisibility(View.VISIBLE);
        if (adapter == null) {
            list = new ArrayList<>();
            list.clear();
            list.addAll(datas);

            adapter = new CtExpRecAdapter(getContext(), list);
            recyclerView.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            recyclerView.setAdapter(adapter);
            adapter.setCommonRvListener((position, o) -> {
                start(GeneralCtDetailFragment.class);
            });
        } else {
            list.clear();
            list.addAll(datas);
            adapter.notifyDataSetChanged();
        }
        experienceTitleTv.setText(String.format(getString(R.string.performancerecord), String.valueOf(list.size())));
    }

    @OnClick({R.id.tab1, R.id.tab2, R.id.impeachment_btn})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.tab1:
                selectDetail();
                break;
            case R.id.tab2:
                selectExperience();
                break;
            case R.id.impeachment_btn:
                Bundle bundle = new Bundle();
                bundle.putString("cid", cid);
                start(ImpeachmentFragment.class, bundle);
                break;
        }

    }

    private String cid;
    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getCouncilInfo":
                setBaseInfo((CtDetailBean) baseEntity);
                setCtInfo((CtDetailBean) baseEntity);
                setCtRecord((CtDetailBean) baseEntity);
                break;
        }
    }


    @BindView(R.id.name)
    TextView name;
    @BindView(R.id.head_ic)
    AppCompatImageView headIc;
    @BindView(R.id.location)
    TextView location;
    @BindView(R.id.vote_count)
    TextView currentVotes;
    @BindView(R.id.impeachment_count)
    TextView impeachmentCount;
    @BindView(R.id.progress)
    CircleProgressView progress;
    private void setBaseInfo(CtDetailBean ctDetailBean) {
        CtDetailBean.DataBean dataBean = ctDetailBean.getData();
        name.setText(dataBean.getDidName());
        cid = dataBean.getCid();
        GlideApp.with(getContext()).load(dataBean.getAvatar()).error(R.mipmap.icon_ela).circleCrop().into(headIc);
        location.setText(AppUtlis.getLoc(getContext(), String.valueOf(dataBean.getLocation())));
        BigDecimal gress = new BigDecimal(dataBean.getImpeachmentVotes()).divide(new BigDecimal(dataBean.getImpeachmentThroughVotes())).multiply(new BigDecimal(100));
        progress.setProgress(gress.floatValue());
        currentVotes.setText(String.valueOf(dataBean.getImpeachmentVotes()));
        impeachmentCount.setText(String.valueOf(dataBean.getImpeachmentThroughVotes()));
    }

    @BindView(R.id.did)
    TextView didTv;
    @BindView(R.id.website)
    TextView website;
    @BindView(R.id.introduction)
    TextView introduction;
    private void setCtInfo(CtDetailBean ctDetailBean) {
        if(null == ctDetailBean) return;
        CtDetailBean.DataBean dataBean = ctDetailBean.getData();
        didTv.setText(dataBean.getDid());
        website.setText(dataBean.getAddress());
        introduction.setText(dataBean.getIntroduction());
    }

    @SuppressLint("DefaultLocale")
    private void setCtRecord(CtDetailBean ctDetailBean) {
        if(null == ctDetailBean) return;
        List<CtDetailBean.Term> terms = ctDetailBean.getData().getTerm();
        setRecyclerView(terms);
    }

}
