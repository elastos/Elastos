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
        selectCtList();

    }


    private void selectCtList() {
        line1.setVisibility(View.VISIBLE);
        line2.setVisibility(View.GONE);
        ctTitleTv.setTextColor(getResources().getColor(R.color.whiter));
        secretaryTitleTv.setTextColor(getResources().getColor(R.color.whiter50));
        generalLayout.setVisibility(View.VISIBLE);
        secretaryLayout.setVisibility(View.GONE);
    }

    private void selectSecretaryList() {
        line1.setVisibility(View.GONE);
        line2.setVisibility(View.VISIBLE);
        ctTitleTv.setTextColor(getResources().getColor(R.color.whiter50));
        secretaryTitleTv.setTextColor(getResources().getColor(R.color.whiter));
        generalLayout.setVisibility(View.GONE);
        secretaryLayout.setVisibility(View.VISIBLE);
    }

    private void refreshGeneralRv(List<CtListBean.Council> datas) {
        if (generalAdapter == null) {
            generalList = new ArrayList<>();
            generalList.clear();
            generalList.addAll(datas);

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
            generalList.clear();
            generalList.addAll(datas);
            generalAdapter.notifyDataSetChanged();
        }
    }

    private void refreshSecretaryRv(List<CtListBean.Secretariat> datas) {
        if (secretaryAdapter == null) {
            secretaryList = new ArrayList<>();
            secretaryList.clear();
            secretaryList.addAll(datas);

            secretaryAdapter = new SecretaryCtRecAdapter(getContext(), secretaryList);
            secretaryRv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            secretaryRv.setAdapter(secretaryAdapter);
            secretaryAdapter.setCommonRvListener((position, o) -> {
                start(SecretaryCtDetailFragment.class);
            });

        } else {
            secretaryList.clear();
            secretaryList.addAll(datas);
            secretaryAdapter.notifyDataSetChanged();
        }
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
        if(methodName.equals("getCouncilList")) {
            setGeneralRec((CtListBean) baseEntity);
            setSecretaryRec((CtListBean) baseEntity);
        }
    }

    private void setSecretaryRec(CtListBean ctListBean) {
        if(null == ctListBean) return;
        List<CtListBean.Secretariat> datas = ctListBean.getData().getSecretariat();
        if(datas==null || datas.size()<=0) return;
        refreshSecretaryRv(datas);
    }

    private void setGeneralRec(CtListBean ctListBean) {
        if(null == ctListBean) return;
        List<CtListBean.Council> datas = ctListBean.getData().getCouncil();
        if(datas==null || datas.size()<=0) return;
        refreshGeneralRv(datas);
    }
}
