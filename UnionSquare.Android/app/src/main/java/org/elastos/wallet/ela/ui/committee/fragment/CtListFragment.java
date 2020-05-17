package org.elastos.wallet.ela.ui.committee.fragment;

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
import org.elastos.wallet.ela.ui.committee.adaper.GeneralCtRecAdapter;
import org.elastos.wallet.ela.ui.committee.adaper.SecretaryCtRecAdapter;
import org.elastos.wallet.ela.ui.committee.bean.CtListBean;
import org.elastos.wallet.ela.ui.committee.presenter.CtListPresenter;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * Committee and secretary general list container
 */
public class CtListFragment extends BaseFragment implements NewBaseViewData {
    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.ct_tv)
    TextView ctTitleTv;
    @BindView(R.id.line1)
    View line1;
    @BindView(R.id.secretary_tv)
    TextView secretaryTitleTv;
    @BindView(R.id.line2)
    View line2;
    @BindView(R.id.ct_general_layout)
    View generalLayout;
    @BindView(R.id.ct_secretary_layout)
    View secretaryLayout;

    @BindView(R.id.general_rv)
    RecyclerView generalRv;
    GeneralCtRecAdapter generalAdapter;
    List<CtListBean.Council> generalList;

    @BindView(R.id.secretary_rv)
    RecyclerView secretaryRv;
    SecretaryCtRecAdapter secretaryAdapter;
    List<CtListBean.Secretariat> secretaryList;

    private CtListPresenter presenter;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_list;
    }

    private int index;
    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        index = data.getInt("index");
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, String.format(getString(R.string.actmember), String.valueOf(index)));

        presenter = new CtListPresenter();
        presenter.getCouncilList(this, String.valueOf(index));
        generalRockData();
        secretaryRockData();

        selectCtList();

    }


    private void selectCtList() {
        line1.setVisibility(View.VISIBLE);
        line2.setVisibility(View.GONE);
        ctTitleTv.setTextColor(getResources().getColor(R.color.whiter));
        secretaryTitleTv.setTextColor(getResources().getColor(R.color.whiter50));
        generalLayout.setVisibility(View.VISIBLE);
        secretaryLayout.setVisibility(View.GONE);
        refreshGeneralRv();
    }

    private void selectSecretaryList() {
        line1.setVisibility(View.GONE);
        line2.setVisibility(View.VISIBLE);
        ctTitleTv.setTextColor(getResources().getColor(R.color.whiter50));
        secretaryTitleTv.setTextColor(getResources().getColor(R.color.whiter));
        generalLayout.setVisibility(View.GONE);
        secretaryLayout.setVisibility(View.VISIBLE);
        refreshSecretaryRv();
    }

    private void refreshGeneralRv() {
        if (generalAdapter == null) {
            generalAdapter = new GeneralCtRecAdapter(getContext(), generalList);
            generalRv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            generalRv.setAdapter(generalAdapter);
            generalAdapter.setCommonRvListener((position, o) -> {
                Bundle bundle = new Bundle();
                bundle.putString("id", String.valueOf(index));
                bundle.putString("did", generalList.get(position).getDid());
                start(GeneralCtDetailFragment.class, bundle);
            });

        } else {
            generalAdapter.notifyDataSetChanged();
        }
    }

    private void refreshSecretaryRv() {
        if (secretaryAdapter == null) {
            secretaryAdapter = new SecretaryCtRecAdapter(getContext(), secretaryList);
            secretaryRv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            secretaryRv.setAdapter(secretaryAdapter);
            secretaryAdapter.setCommonRvListener((position, o) -> {
                start(SecretaryCtDetailFragment.class);
            });

        } else {
            secretaryAdapter.notifyDataSetChanged();
        }
    }

    private void secretaryRockData() {
        secretaryList = new ArrayList<>();
        CtListBean.Secretariat SecretaryCtBean1 = new CtListBean.Secretariat();
        SecretaryCtBean1.setDidName("Feng Zhang1");
        SecretaryCtBean1.setLocation(86);
        secretaryList.add(SecretaryCtBean1);

        CtListBean.Secretariat SecretaryCtBean2 = new CtListBean.Secretariat();
        SecretaryCtBean2.setDidName("Feng Zhang2");
        SecretaryCtBean2.setLocation(86);
        secretaryList.add(SecretaryCtBean2);

        CtListBean.Secretariat SecretaryCtBean3 = new CtListBean.Secretariat();
        SecretaryCtBean3.setDidName("Feng Zhang3");
        SecretaryCtBean3.setLocation(86);
        secretaryList.add(SecretaryCtBean3);

    }

    private void generalRockData() {
        generalList = new ArrayList<>();
        CtListBean.Council GeneralCtBean1 = new CtListBean.Council();
        GeneralCtBean1.setDidName("Feng Zhang1");
        GeneralCtBean1.setLocation(86);
        GeneralCtBean1.setAvatar("");
        GeneralCtBean1.setStatus("Elected");
        generalList.add(GeneralCtBean1);

        CtListBean.Council GeneralCtBean2 = new CtListBean.Council();
        GeneralCtBean2.setDidName("Feng Zhang1");
        GeneralCtBean2.setLocation(86);
        GeneralCtBean2.setAvatar("");
        GeneralCtBean2.setStatus("Returned");
        generalList.add(GeneralCtBean2);

        CtListBean.Council GeneralCtBean3 = new CtListBean.Council();
        GeneralCtBean3.setDidName("Feng Zhang1");
        GeneralCtBean3.setLocation(86);
        GeneralCtBean3.setAvatar("");
        GeneralCtBean3.setStatus("Impeached");
        generalList.add(GeneralCtBean3);

        CtListBean.Council GeneralCtBean4 = new CtListBean.Council();
        GeneralCtBean4.setDidName("Feng Zhang1");
        GeneralCtBean4.setLocation(86);
        GeneralCtBean4.setAvatar("");
        GeneralCtBean4.setStatus(null);
        generalList.add(GeneralCtBean4);
    }

    @OnClick({R.id.ct_layout, R.id.secretary_layout})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.ct_layout:
                selectCtList();
                break;
            case R.id.secretary_layout:
                selectSecretaryList();
                break;
        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getSuggestion":
                setGeneralRec((CtListBean) baseEntity);
                setSecretaryRec((CtListBean) baseEntity);
                break;
        }
    }

    private void setSecretaryRec(CtListBean ctListBean) {
        if(null == ctListBean) return;
        List<CtListBean.Secretariat> datas = ctListBean.getData().getSecretariat();
        if(datas==null || datas.size()<=0) return;
        if(null == secretaryList) {
            secretaryList = new ArrayList<>();
        } else {
            secretaryList.clear();
        }

        for(CtListBean.Secretariat data : datas) {
            secretaryList.add(data);
        }
        refreshSecretaryRv();
    }

    private void setGeneralRec(CtListBean ctListBean) {
        if(null == ctListBean) return;
        List<CtListBean.Council> datas = ctListBean.getData().getCouncil();
        if(datas==null || datas.size()<=0) return;
        if(null == generalList) {
            generalList = new ArrayList<>();
        } else {
            generalList.clear();
        }
        for(CtListBean.Council data : datas) {
            generalList.add(data);
        }
        refreshGeneralRv();
    }
}
