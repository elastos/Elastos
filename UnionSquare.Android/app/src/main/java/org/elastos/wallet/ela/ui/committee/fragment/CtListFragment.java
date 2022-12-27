/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
import org.elastos.wallet.ela.utils.SPUtil;

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
        String stage =  index + "";
        int Language = new SPUtil(getContext()).getLanguage();
        if (Language != 0) {
            switch (index) {
                case 1:
                    stage = "1st";
                    break;
                case 2:
                    stage = "2nd";
                    break;
                case 3:
                    stage = "3rd";
                    break;
                default:
                    stage = stage + "th";
            }
        }
        setToobar(toolbar, toolbarTitle, String.format(getString(R.string.actmember), stage));

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
                bundle.putString("status", generalList.get(position).getStatus());
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
                Bundle bundle = new Bundle();
                bundle.putString("id", String.valueOf(index));
                bundle.putString("did", secretaryList.get(position).getDid());
                start(SecretaryCtDetailFragment.class, bundle);
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
