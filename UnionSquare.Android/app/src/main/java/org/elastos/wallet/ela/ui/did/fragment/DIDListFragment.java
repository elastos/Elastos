package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.fragment.TransferDetailFragment;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.did.adapter.DIDRecordRecAdapetr;

import butterknife.BindView;
import butterknife.OnClick;

public class DIDListFragment extends BaseFragment implements NewBaseViewData, CommonRvListener {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.line_tab1)
    View lineTab1;
    @BindView(R.id.tv_tab1)
    TextView tvTab1;

    @BindView(R.id.line_tab2)
    View lineTab2;
    @BindView(R.id.tv_tab2)
    TextView tvTab2;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.rv1)
    RecyclerView rv1;
    private DIDRecordRecAdapetr adapter1;
    private DIDRecordRecAdapetr adapter;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_did_list;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText("DID");

    }


    @OnClick({R.id.ll_tab1, R.id.ll_tab2})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.ll_tab1:
                lineTab1.setVisibility(View.VISIBLE);
                lineTab2.setVisibility(View.GONE);
                tvTab1.setTextColor(getResources().getColor(R.color.whiter));
                tvTab2.setTextColor(getResources().getColor(R.color.whiter50));
                rv.setVisibility(View.VISIBLE);
                rv1.setVisibility(View.GONE);
                break;
            case R.id.ll_tab2:
                lineTab1.setVisibility(View.GONE);
                lineTab2.setVisibility(View.VISIBLE);
                tvTab1.setTextColor(getResources().getColor(R.color.whiter50));
                tvTab2.setTextColor(getResources().getColor(R.color.whiter));
                rv.setVisibility(View.GONE);
                rv1.setVisibility(View.VISIBLE);
                break;
        }
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getAllSubWallets":

                break;

        }

    }

  /*  private void setRecycleView(TransferRecordEntity entity) {
        List<TransferRecordEntity.TransactionsBean> data = entity.getTransactions();
        if (startCount == 0 && (data == null || data.size() == 0)) {
            rv.setVisibility(View.GONE);
            tvRecordBg.setVisibility(View.VISIBLE);
            return;
        } else {
            rv.setVisibility(View.VISIBLE);
            tvRecordBg.setVisibility(View.GONE);
        }

        if (startCount == 0) {
            if (list == null) {
                list = new ArrayList<>();
            } else {
                list.clear();
            }
        } else if (data == null || data.size() == 0) {
            showToastMessage(getString(R.string.loadall));
            return;
        }
        list.addAll(data);
        if (adapter == null) {
            adapter = new DIDRecordRecAdapetr(getContext(), list, chainId);
            rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv.setAdapter(adapter);
            adapter.setCommonRvListener(this);

        } else {
            adapter.notifyDataSetChanged();
        }
        startCount += data.size();
    }

    private void setRecycleView1(TransferRecordEntity entity) {
        List<TransferRecordEntity.TransactionsBean> data = entity.getTransactions();
        if (startCount1 == 0 && (data == null || data.size() == 0)) {
            rv1.setVisibility(View.GONE);
            tvEarnBg.setVisibility(View.VISIBLE);
            return;
        } else {
            rv1.setVisibility(View.VISIBLE);
            tvEarnBg.setVisibility(View.GONE);
        }

        if (startCount1 == 0) {
            if (list1 == null) {
                list1 = new ArrayList<>();
            } else {
                list1.clear();
            }
        } else if (data == null || data.size() == 0) {
            showToastMessage(getString(R.string.loadall));
            return;
        }
        list1.addAll(data);
        if (adapter1 == null) {
            adapter1 = new DIDRecordRecAdapetr(getContext(), list1, chainId);
            rv1.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv1.setAdapter(adapter1);
            adapter1.setCommonRvListener(this);

        } else {
            adapter1.notifyDataSetChanged();
        }
        startCount1 += data.size();
    }*/

    @Override
    public void onRvItemClick(int position, Object o) {
        Bundle bundle = new Bundle();

        if (rv1.getVisibility() == View.VISIBLE) {
            bundle.putInt("recordType", 1);//发布记录
        } else {
            bundle.putInt("recordType", 0);//草稿记录
        }

        start(TransferDetailFragment.class, bundle);
    }
}
