package org.elastos.wallet.ela.ui.committee.fragment;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.committee.adaper.CtExpRecAdapter;
import org.elastos.wallet.ela.ui.committee.bean.ExperienceBean;
import org.elastos.wallet.ela.ui.committee.bean.CtDetailBean;
import org.elastos.wallet.ela.ui.committee.presenter.GeneralDetailPresenter;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.view.CircleProgressView;

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
    CtExpRecAdapter adapter;
    List<ExperienceBean> list;

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
        return R.layout.fragment_ct_general_detail;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getContext().getString(R.string.ctdetail));
        progress.setProgress(50);
        presenter = new GeneralDetailPresenter();
        presenter.getCouncilInfo(this, id, did);
//        rockData();
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

    private void setRecyclerView() {
        if (adapter == null) {
            adapter = new CtExpRecAdapter(getContext(), list);
            recyclerView.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            recyclerView.setAdapter(adapter);
            adapter.setCommonRvListener((position, o) -> {
                start(GeneralCtDetailFragment.class);
            });

        } else {
            adapter.notifyDataSetChanged();
        }
    }

    private void rockData() {
        list = new ArrayList<>();
        ExperienceBean ExperienceBean1 = new ExperienceBean();
        ExperienceBean1.setTitle("马尔代夫联谊团建活动");
        ExperienceBean1.setSubTitle("#99 2019.07.01 疯狂的茄子 已否决");
        ExperienceBean1.setType(1);
        list.add(ExperienceBean1);

        ExperienceBean ExperienceBean2 = new ExperienceBean();
        ExperienceBean2.setTitle("What should belongs in a normal CRC proposal?");
        ExperienceBean2.setSubTitle("#99 2019.07.01 Yipeng Su 公示中");
        ExperienceBean2.setType(2);
        list.add(ExperienceBean2);

        ExperienceBean ExperienceBean3 = new ExperienceBean();
        ExperienceBean3.setTitle("建议大象钱包团队，把cr网站加入到大象钱包中。");
        ExperienceBean3.setSubTitle("#99 2019.07.01 Yipeng Su 公示中");
        ExperienceBean3.setType(0);
        list.add(ExperienceBean3);

        ExperienceBean ExperienceBean4 = new ExperienceBean();
        ExperienceBean4.setTitle("Community Management Team Proposal");
        ExperienceBean4.setSubTitle("#99 2019.07.01 Yipeng Su 公示中");
        ExperienceBean4.setType(2);
        list.add(ExperienceBean4);

        setRecyclerView();
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
                start(ImpeachmentFragment.class);
                break;
        }

    }

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
    @BindView(R.id.location)
    TextView location;
    @BindView(R.id.vote_count)
    TextView currentVotes;
    @BindView(R.id.impeachment_count)
    TextView impeachmentCount;
    @BindView(R.id.progress)
    CircleProgressView progress;
    private void setBaseInfo(CtDetailBean ctDetailBean) {
        CtDetailBean.DataBean dataBean = ctDetailBean.getData().get(0);
        name.setText(dataBean.getDidName());
        location.setText(AppUtlis.getLoc(getContext(), String.valueOf(dataBean.getLocation())));
        currentVotes.setText(String.valueOf(dataBean.getImpeachmentVotes()));
        impeachmentCount.setText(String.valueOf(dataBean.getImpeachmentHeight()));
    }

    @BindView(R.id.did)
    TextView didTv;
    @BindView(R.id.website)
    TextView website;
    @BindView(R.id.introduction)
    TextView introduction;
    private void setCtInfo(CtDetailBean ctDetailBean) {
        if(null == ctDetailBean) return;
        CtDetailBean.DataBean dataBean = ctDetailBean.getData().get(0);
        didTv.setText(dataBean.getDid());
        website.setText(dataBean.getAddress());
        introduction.setText(dataBean.getIntroduction());
    }

    @SuppressLint("DefaultLocale")
    private void setCtRecord(CtDetailBean ctDetailBean) {
        if(null == ctDetailBean) return;
        List<CtDetailBean.Term> terms = ctDetailBean.getData().get(0).getTerm();
        for(CtDetailBean.Term term : terms) {
            ExperienceBean ExperienceBean = new ExperienceBean();
            ExperienceBean.setTitle(term.getDidName());
            ExperienceBean.setSubTitle(String.format("#%1$d %2$s %3$s %4$s", term.getId(),
                    DateUtil.formatTimestamp(String.valueOf(term.getCreatedAt()), "yyyy.MM.dd"),
                    term.getDidName(), term.getStatus()));
            ExperienceBean.setType(1);
            list.add(ExperienceBean);
        }
    }
}
